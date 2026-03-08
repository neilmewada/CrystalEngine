---
  Setup: What the Layer Tree Looks Like

  Given a widget tree like:

  Window (root)
    Panel (Opacity=0.9)          ← compositing boundary
      BlurCard (ForceEffect)     ← effect boundary
        Icon
        Label
      Button                     ← paint boundary (ForceRepaintBoundary)
        Text
      Checkbox

  The layer tree built from this:

  FLayerNode [Window]               ← root, always exists
    FCompositingLayerNode [Panel]
      FEffectLayerNode [BlurCard]
        FLayerNode [Button]
      (Icon, Label, Checkbox paint into their ancestor's node)

  Widgets with no boundary paint into their nearest ancestor layer node's draw list. No node is created for them.

  ---
  Phase 1 — Layer Tree Sync

  This runs after layout, before painting. It's a diff walk — not a full rebuild every frame.

  The Walk

  FLayerTree does a depth-first traversal of the widget tree, carrying a pointer to the "current parent layer node." For each widget it asks three questions via the boundary query
  methods:

  TraverseWidget(widget, parentNode):

    thisNode = parentNode       ← default: this widget paints into parent's node

    if widget.IsEffectBoundary():
        thisNode = FindOrCreate<FEffectLayerNode>(widget, parentNode)
    elif widget.IsCompositingBoundary():
        thisNode = FindOrCreate<FCompositingLayerNode>(widget, parentNode)
    elif widget.IsPaintBoundary():
        thisNode = FindOrCreate<FLayerNode>(widget, parentNode)

    for each child of widget:
        TraverseWidget(child, thisNode)

  FindOrCreate checks widgetToNode map first. If a node already exists for this widget, it verifies the node is still the right type. If the type changed (e.g. opacity dropped below
  1.0, so it's now a compositing boundary), it replaces it.

  What Triggers a Sync

  Not every frame. The sync runs only when the layerTreeDirty flag is set on the surface. That flag is set when:

  - A widget is added or removed from the widget tree
  - A widget's Opacity crosses the 1.0 threshold in either direction
  - A ForceXxxBoundary flag changes on any widget

  What the Sync Does NOT Do

  - It does not look at colors, text, brushes, positions — those are paint concerns
  - It does not allocate RTs (that's the compositor's job, done lazily on first use)
  - It does not clear dirty flags

  Output of Phase 1

  A stable layer tree that correctly mirrors the widget tree's boundary structure. The widgetToNode map is up to date. Every widget that is a boundary has a corresponding layer node;
   every other widget has no node and paints into an ancestor's.

  ---
  Phase 2 — Rendering

  This has two sub-passes that run back to back.

  ---
  Sub-pass A — Paint Pass

  Goal: Re-record draw commands for every dirty layer node.

  The surface maintains dirtyPaintNodes — a set of FLayerNode* populated by OnWidgetPaintDirty. Before painting, it prunes this set: if a node's ancestor is also dirty, remove the
  descendant (the ancestor repaint will cover it anyway).

  For each remaining dirty node:

  PaintNode(layerNode):
      painter = FPainter(layerNode->drawList)
      painter.Begin()

      PaintWidgetSubtree(layerNode->owningWidget, painter)

      painter.End()
      layerNode->needsRepaint = false
      layerNode->needsRecomposite = true   ← content changed, must re-composite

  PaintWidgetSubtree(widget, painter):
      if widget is hidden: return

      painter.PushCoordinateSpace(widget->layoutPosition + widget->Transform)

      widget->OnPaint(painter)   ← widget draws its own content (background, border, etc.)

      for each child of widget:
          childNode = widgetToNode[child]   ← does this child own a layer node?

          if childNode exists:
              // Child is a boundary — do NOT recurse into it here.
              // It either has its own dirty entry, or its RT is still valid.
              // The composite pass will handle it.
              continue
          else:
              // Child paints inline into this node's draw list
              PaintWidgetSubtree(child, painter)

      painter.PopCoordinateSpace()

  Key point: the paint pass stops at layer node boundaries. It never crosses into a child layer node's subtree. Each node's draw list contains only the draw commands for widgets that
   belong to that node — not its children nodes.

  FPainter here is just a draw list recorder. It writes FUIDrawCommand entries into the node's draw list. No GPU calls happen yet.

  ---
  Sub-pass B — Composite Pass

  Goal: Produce the final image by walking the layer tree bottom-up and blending everything together.

  The compositor walks depth-first, children before parents (post-order). For each node it composites the node's children into itself, then yields itself to its parent.

  Composite(node, targetRT):

      // First, recurse — composite children into this node's RT
      for each child of node:
          Composite(child, node->renderTarget)   ← or screen if node is root

      // Then, composite this node itself into the target
      if node is FEffectLayerNode:
          CompositeEffectNode(node, targetRT)
      elif node is FCompositingLayerNode:
          CompositeCompositingNode(node, targetRT)
      else:  // FLayerNode (plain paint node)
          SubmitDrawList(node->drawList, targetRT)

  Plain FLayerNode (paint boundary)

  No RT. Just submit its draw list directly into whatever target is being rendered into. This is effectively free — the draw list is already recorded, just needs to be submitted to
  the GPU.

  FCompositingLayerNode (compositing boundary)

  CompositeCompositingNode(node, targetRT):
      if node->needsRecomposite:
          // Render node's own draw list into its RT
          GPU: RenderDrawList(node->drawList → node->renderTarget)
          node->needsRecomposite = false

      // Blend the node's RT into the parent target with opacity/transform
      GPU: BlendTexture(node->renderTarget → targetRT,
                        opacity: node->owningWidget->Opacity,
                        transform: node->owningWidget->Transform)

  Note: if only Opacity changed (no repaint), needsRepaint was never set, needsRecomposite was set directly. The GPU never re-records draw commands — it just re-blends the cached RT
  with the new opacity value. Opacity animations are re-composite only, not repaint.

  FEffectLayerNode (effect boundary)

  This is the most involved:

  CompositeEffectNode(node, targetRT):
      // Step 1: Capture the background
      // Everything that has been composited into targetRT *so far* (i.e., everything
      // rendered behind this node in draw order) gets copied/blitted into
      // node->backgroundCapture RT.
      GPU: CopyRegion(targetRT → node->backgroundCapture, region: node->bounds)

      // Step 2: Render the node's own draw list into its RT
      if node->needsRecomposite:
          GPU: RenderDrawList(node->drawList → node->renderTarget)
          node->needsRecomposite = false

      // Step 3: Apply effect using both RTs as input
      // The effect shader samples backgroundCapture for blur/distortion,
      // and composites node->renderTarget (own content) on top.
      GPU: RunEffectShader(
               background: node->backgroundCapture,
               content:    node->renderTarget,
               output:     targetRT,
               params:     blurRadius, etc.)

  This works because the composite pass is post-order — by the time we reach this node, everything drawn below it in z-order has already been composited into targetRT. The copy at
  step 1 captures exactly the right background pixels.

  ---
  The Full Frame Timeline

  Layout pass
    └─ Widget tree: MeasureContent + ArrangeContent for dirty layout roots

  Layer tree sync  (only if layerTreeDirty)
    └─ Diff walk: create/destroy/retype layer nodes to match widget boundaries

  Paint pass  (only for nodes in dirtyPaintNodes)
    └─ Per dirty FLayerNode:
         FPainter records draw commands into node's draw list
         Stops recursion at child layer node boundaries
         Sets needsRecomposite = true on the node

  Composite pass  (every frame, but mostly GPU blending of cached RTs)
    └─ Post-order walk of layer tree:
         FLayerNode        → submit draw list inline
         FCompositingNode  → render draw list to RT (if dirty), blend RT into parent
         FEffectNode       → capture background, render to RT, run effect shader, blend

  ---
  Why This Division Works Well

  The layer tree sync ensures the compositor always has an accurate structural picture before any rendering begins. The paint pass is purely CPU-side draw command recording — no GPU
  involvement. The composite pass is purely GPU work against stable, pre-built data. The three concerns never interleave.