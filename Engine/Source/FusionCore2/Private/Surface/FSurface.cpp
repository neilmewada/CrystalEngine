#include "FusionCore.h"

namespace CE
{

    FSurface::FSurface()
    {
        layerTree = CreateDefaultSubobject<FLayerTree>("LayerTree");
    }

    void FSurface::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

	    for (int i = 0; i < viewConstantBuffers.GetSize(); ++i)
	    {
            delete viewConstantBuffers[i]; viewConstantBuffers[i] = nullptr;
	    }

        for (int i = 0; i < quadBuffersPerImage.GetSize(); i++)
        {
            delete quadBuffersPerImage[i]; quadBuffersPerImage[i] = nullptr;
        }

        for (u32 i = 0; i < renderSnapshots.GetSize(); i++)
        {
            renderSnapshots[i] = nullptr;
        }

        delete viewSrg; viewSrg = nullptr;

        Shutdown();
    }

    void FSurface::GetDrawListMask(RHI::DrawListMask& drawListMask)
    {
		if (drawListTag.IsValid())
        {
            drawListMask.Set(drawListTag);
        }
    }

    void FSurface::AddChildSurface(Ref<FSurface> childSurface)
    {
		if (childrenSurfaces.Exists(childSurface))
        {
            return;
        }

		childrenSurfaces.Add(childSurface);
		childSurface->parentSurface = this;
    }

    void FSurface::RemoveChildSurface(Ref<FSurface> childSurface)
    {
        if (childSurface->parentSurface == this)
        {
            childSurface->parentSurface = nullptr;
        }

        childrenSurfaces.Remove(childSurface);
    }

    bool FSurface::IsNativeSurface()
    {
		return IsOfType<FNativeSurface>();
    }

    void FSurface::SetOwningWidget(Ref<FWidget> widget)
    {
		if (rootWidget == widget)
            return;

		if (rootWidget)
        {
            rootWidget->SetParentSurfaceRecursive(nullptr);
        }

		rootWidget = widget;

        if (rootWidget)
		{
			rootWidget->SetParentSurfaceRecursive(this);

            AddPendingLayoutRoot(rootWidget);

            rootWidget->UpdateBoundaryFlags();

            rootWidget->MarkLayoutDirty();
            rootWidget->MarkPaintDirty();
		}
    }

    Ref<FStyleSet> FSurface::GetStyleSet()
    {
        if (styleSet.IsValid())
			return styleSet;
        if (Ref<FSurface> parent = parentSurface.Lock())
        {
            return parent->GetStyleSet();
        }
        return FApplication::Get()->GetDefaultStyleSet();
    }

    void FSurface::MarkLayerTreeDirty()
    {
        layerTree->MarkSyncNeeded();
    }

    void FSurface::AddPendingLayoutRoot(Ref<FWidget> layoutRoot)
    {
        if (!layoutRoot)
            return;

		if (pendingLayoutRootIds.Exists(layoutRoot->GetUuid()))
            return;

		pendingLayoutRoots.Add(layoutRoot);
        pendingLayoutRootIds.Add(layoutRoot->GetUuid());
    }

    void FSurface::MarkRootLayoutDirty()
    {
        if (!rootWidget)
            return;

        AddPendingLayoutRoot(rootWidget);

        rootWidget->MarkLayoutDirty();
        rootWidget->MarkPaintDirty();
    }

    void FSurface::DispatchSurfaceUnfocusEvent()
    {
        ZoneScoped;

        if (Ref<FWidget> widget = curFocusedWidget.Lock())
        {
            FFocusEvent focusEvent{};
            focusEvent.eventType = FEventType::FocusChanged;
            focusEvent.gotFocus = false;
            focusEvent.focusedWidget = nullptr;
            focusEvent.sender = widget;

            widget->OnFocusChanged(focusEvent);
        }
    }

    void FSurface::DispatchSurfaceFocusEvent()
    {
        ZoneScoped;

        curFocusedWidget = nextFocusWidget;

        if (Ref<FWidget> widget = curFocusedWidget.Lock())
        {
            FFocusEvent focusEvent{};
            focusEvent.eventType = FEventType::FocusChanged;
            focusEvent.gotFocus = true;
            focusEvent.focusedWidget = widget;
            focusEvent.sender = widget;

            widget->OnFocusChanged(focusEvent);
        }
    }

    static void BuildHoverStack(FWidget* leaf, Array<FWidget*>& outStack)
    {
        FWidget* w = leaf;
        while (w)
        {
            outStack.Add(w);
            w = w->GetParentWidget().Get();
        }
    }

    void FSurface::DispatchMouseEvents()
    {
        ZoneScoped;

        Ref<FEventService> eventService = FApplication::Get()->GetService<FEventService>();
        if (!eventService)
            return;

        // - Mouse Pos

        Vec2 screenMousePos = eventService->GetScreenMousePos();
        Vec2 prevScreenMousePos = eventService->GetPrevScreenMousePos();
        Vec2 wheelDelta = eventService->GetMouseWheelDelta();

        Vec2 surfaceMousePos = ScreenToSurfacePoint(screenMousePos);
        Vec2 prevSurfaceMousePos = ScreenToSurfacePoint(prevScreenMousePos);
        Vec2 surfaceMouseDelta = surfaceMousePos - prevSurfaceMousePos;

        // - Mouse Buttons

        MouseButtonMask curButtonMask = MouseButtonMask::None;
        if (InputManager::IsMouseButtonHeld(MouseButton::Left))
        {
            curButtonMask |= MouseButtonMask::Left;
        }
        if (InputManager::IsMouseButtonHeld(MouseButton::Right))
        {
            curButtonMask |= MouseButtonMask::Right;
        }
        if (InputManager::IsMouseButtonHeld(MouseButton::Middle))
        {
            curButtonMask |= MouseButtonMask::Middle;
        }
        if (InputManager::IsMouseButtonHeld(MouseButton::Button4))
        {
            curButtonMask |= MouseButtonMask::Button4;
        }
        if (InputManager::IsMouseButtonHeld(MouseButton::Button5))
        {
            curButtonMask |= MouseButtonMask::Button5;
        }

        // - Key Modifiers

        keyModifierStates = KeyModifier::None;

        Enum* keyModifierEnum = GetStaticEnum<KeyModifier>();

        for (int i = 0; i < keyModifierEnum->GetConstantsCount(); ++i)
        {
            if (InputManager::TestModifiers((KeyModifier)keyModifierEnum->GetConstant(i)->GetValue()))
            {
                keyModifierStates |= (KeyModifier)keyModifierEnum->GetConstant(i)->GetValue();
            }
        }

        bool mouseInSurface = Rect::FromSize(Vec2(), availableSize).Contains(surfaceMousePos);

        Ref<FWidget> captured = capturedWidget.Lock();

        FWidget* hitWidget = mouseInSurface ? HitTestWidget(surfaceMousePos) : nullptr;

        FWidget* hoveredWidget = captured.IsValid() ? captured.Get() : hitWidget;

        Array<FWidget*> newHoverStack;
        BuildHoverStack(hoveredWidget, newHoverStack);

        // Mouse Leave
        for (WeakRef<FWidget>& weakWidget : hoveredWidgetStack)
        {
            Ref<FWidget> widget = weakWidget.Lock();
            if (!widget || newHoverStack.Exists(widget.Get()))
                continue;

            FMouseEvent event{};
            event.eventType = FEventType::MouseLeave;
            event.sender = widget;
            event.mousePosition = surfaceMousePos;
            event.prevMousePosition = prevSurfaceMousePos;
            event.wheelDelta = wheelDelta;
            event.isInside = false;
            event.keyModifiers = keyModifierStates;
            
            widget->OnMouseLeave(event);
        }

        // Mouse Enter
        for (int i = newHoverStack.GetSize() - 1; i >= 0; i--)
        {
            FWidget* widget = newHoverStack[i];

            bool wasHovered = hoveredWidgetStack.Exists(widget);
            if (wasHovered)
                continue;

            FMouseEvent event{};
            event.eventType = FEventType::MouseEnter;
            event.sender = widget;
            event.mousePosition = surfaceMousePos;
            event.prevMousePosition = prevSurfaceMousePos;
            event.isInside = true;
            event.keyModifiers = keyModifierStates;

            widget->OnMouseEnter(event);
        }

        hoveredWidgetStack.Clear();
        for (FWidget* widget : newHoverStack)
            hoveredWidgetStack.Add(widget);

        if (hoveredWidget)
        {
            constexpr auto epsilon = std::numeric_limits<float>::epsilon();

            // Mouse Move
	        if (abs(surfaceMouseDelta.x) >= epsilon || abs(surfaceMouseDelta.y) >= epsilon)
	        {
                FMouseEvent event{};
                event.eventType = FEventType::MouseMove;
                event.sender = hoveredWidget;
                event.mousePosition = surfaceMousePos;
                event.prevMousePosition = prevSurfaceMousePos;
                event.buttons = curButtonMask;
                event.isInside = true;
                event.keyModifiers = keyModifierStates;

                FEventReply reply = hoveredWidget->OnMouseMove(event);
                ProcessReply(hoveredWidget, reply);
	        }

            // Mouse Wheel
            if (abs(wheelDelta.x) >= epsilon || abs(wheelDelta.y) >= epsilon)
            {
                for (WeakRef<FWidget>& weakWidget : hoveredWidgetStack)
                {
                    Ref<FWidget> widget = weakWidget.Lock();
                    if (!widget) continue;

                    FMouseEvent event{};
                    event.eventType = FEventType::MouseWheel;
                    event.sender = hoveredWidget;
                    event.mousePosition = surfaceMousePos;
                    event.prevMousePosition = prevSurfaceMousePos;
                    event.wheelDelta = wheelDelta;
                    event.buttons = curButtonMask;
                    event.isInside = true;
                    event.keyModifiers = keyModifierStates;

                    FEventReply reply = widget->OnMouseWheel(event);
                    ProcessReply(widget.Get(), reply);
                    if (reply.IsHandled())
                        break;
                }
            }
        }

        constexpr MouseButton kButtons[] = {
        	MouseButton::Left, MouseButton::Right, MouseButton::Middle,
        	MouseButton::Button4, MouseButton::Button5
        };
        constexpr MouseButtonMask kMasks[] = {
            MouseButtonMask::Left, MouseButtonMask::Right, MouseButtonMask::Middle,
            MouseButtonMask::Button4, MouseButtonMask::Button5
        };

        // - Mouse Down/Up

        for (int i = 0; i < 5; i++)
        {
            if (InputManager::IsMouseButtonDown(kButtons[i]))
            {
                FWidget* target = captured ? captured.Get() : hitWidget;
                if (target)
                {
                    FMouseEvent downEvent{};
                    downEvent.eventType = FEventType::MouseButtonDown;
                    downEvent.sender = target;
                    downEvent.mousePosition = surfaceMousePos;
                    downEvent.prevMousePosition = prevSurfaceMousePos;
                    downEvent.buttons = kMasks[i];
                    downEvent.isInside = true;
                    downEvent.keyModifiers = keyModifierStates;

                    // Bubble up until handled
                    FWidget* current = target;
                    while (current)
                    {
                        downEvent.sender = current;
                        FWidget* parent = current->GetParentWidget().Get();

                        try
                        {
                            FEventReply reply = current->OnMouseButtonDown(downEvent);
                            ProcessReply(current, reply);
                            if (reply.IsHandled())
                                break;
                        }
                        catch (const Exception& e)
                        {
                            current->SetWidgetFlag(FWidgetFlags::Faulted, true);
                            CE_LOG(Critical, All, "Exception in {}::OnMouseButtonDown(): {}\n{}", current->GetClass()->GetName().GetLastComponent(), e.what(), e.GetStackTraceString(true));

                            if (parent)
                            {
                                parent->MarkLayoutDirty();
                                parent->MarkPaintDirty();
                            }
                            else if (rootWidget)
                            {
                                rootWidget->MarkLayoutDirty();
                                rootWidget->MarkPaintDirty();
                            }
                        }

                        current = parent;
                    }

                    pressedWidgetPerButton[i] = target;
                }
            }

            if (InputManager::IsMouseButtonUp(kButtons[i]))
            {
                Ref<FWidget> pressed = pressedWidgetPerButton[i].Lock();
                FWidget* upTarget = captured ? captured.Get()
                    : pressed ? pressed.Get()
                    : nullptr;

                if (upTarget)
                {
                    FMouseEvent upEvent{};
                    upEvent.eventType = FEventType::MouseButtonUp;
                    upEvent.mousePosition = surfaceMousePos;
                    upEvent.prevMousePosition = prevSurfaceMousePos;
                    upEvent.buttons = kMasks[i];
                    upEvent.isInside = (upTarget == hitWidget);
                    upEvent.keyModifiers = keyModifierStates;

                    // Bubble up from the pressed/captured widget
                    FWidget* current = upTarget;
                    while (current)
                    {
                        upEvent.sender = current;
                        FWidget* parent = current->GetParentWidget().Get();
                        
                        try
	                    {
		                    FEventReply reply = current->OnMouseButtonUp(upEvent);
                        	ProcessReply(current, reply);
                        	if (reply.IsHandled())
                        		break;
	                    }
                        catch (const Exception& e)
                        {
                            current->SetWidgetFlag(FWidgetFlags::Faulted, true);
                            CE_LOG(Critical, All, "Exception in {}::OnMouseButtonUp(): {}\n{}", current->GetClass()->GetName().GetLastComponent(), e.what(), e.GetStackTraceString(true));

                            if (parent)
                            {
                                parent->MarkLayoutDirty();
                                parent->MarkPaintDirty();
                            }
                            else if (rootWidget)
                            {
                                rootWidget->MarkLayoutDirty();
                                rootWidget->MarkPaintDirty();
                            }
                        }

                        current = parent;
                    }
                }

                pressedWidgetPerButton[i] = nullptr;
            }
        }

        Ref<FWidget> nextFocus = nextFocusWidget.Lock();
        Ref<FWidget> curFocus = curFocusedWidget.Lock();

        if (nextFocus != curFocus)
        {
            if (curFocus)
            {
                FFocusEvent lostEvent{};
                lostEvent.eventType = FEventType::FocusChanged;
                lostEvent.gotFocus = false;
                lostEvent.focusedWidget = nextFocus;
                lostEvent.sender = curFocus;
                curFocus->OnFocusChanged(lostEvent);
            }

            if (nextFocus)
            {
                FFocusEvent gotEvent{};
                gotEvent.eventType = FEventType::FocusChanged;
                gotEvent.gotFocus = true;
                gotEvent.focusedWidget = nextFocus;
                gotEvent.sender = nextFocus;
                nextFocus->OnFocusChanged(gotEvent);
            }

            curFocusedWidget = nextFocus;
            nextFocusWidget = nullptr;
        }
    }

    void FSurface::DispatchKeyEvents()
    {
        
    }

    void FSurface::ProcessReply(FWidget* sender, const FEventReply& reply)
    {
        switch (reply.GetMouseCaptureOp())
        {
        case FEventReply::MouseCaptureOp::Capture:  capturedWidget = sender; break;
        case FEventReply::MouseCaptureOp::Release:  capturedWidget = nullptr; break;
        default: break;
        }

        if (reply.ShouldFocusSelf())
            nextFocusWidget = sender;
    }

    FWidget* FSurface::HitTestWidget(Vec2 pos, FWidget* widget)
    {
        if (widget == nullptr)
        {
            widget = rootWidget.Get();
            if (!widget) 
                return nullptr;
        }

        if (!widget->IsEnabled() || widget->IsFaulted() || !widget->IsVisible())
            return nullptr;

        // Broad-phase: pos is in widget's own layer space, cachedLayerSpaceAABB is too
        if (!widget->cachedLayerSpaceAABB.Contains(pos))
            return nullptr;

        // Walk children last-to-first (last = painted on top = check first)
        for (int i = (int)widget->GetChildCount() - 1; i >= 0; i--)
        {
            FWidget* child = widget->GetChildAt(i).Get();
            if (!child) continue;

            Vec2 childPos;
            if (child->IsPaintBoundary())
            {
                // Child layer space = parent widget's local space numerically.
                // Convert: parent layer pos → child layer pos via parent widget's transform inverse.
                childPos = widget->cachedLayerSpaceTransform.Inverse().TransformPoint(pos);
            }
            else
            {
                childPos = pos;
            }

            if (FWidget* hit = HitTestWidget(childPos, child))
                return hit;
        }

        // Exact self hit test — convert layer pos → widget local space
        Vec2 localPos = widget->cachedLayerSpaceTransform.Inverse().TransformPoint(pos);
        if (widget->SelfHitTest(localPos))
            return widget;

        return nullptr;
    }

    void FSurface::Initialize()
    {
        for (u32 i = 0; i < viewConstantBuffers.GetSize(); i++)
        {
            RHI::BufferDescriptor desc{};
            desc.name = "FSurface ViewConstants";
            desc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
            desc.bufferSize = sizeof(RPI::PerViewConstants);
            desc.structureByteStride = desc.bufferSize;
            desc.defaultHeapType = RHI::MemoryHeapType::Upload;

            viewConstantBuffers[i] = RHI::gDynamicRHI->CreateBuffer(desc);
        }

        RHI::ShaderResourceGroupLayout viewSrgLayout;
        viewSrgLayout.srgType = RHI::SRGType::PerView;
        viewSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_PerViewData",
                0,
                RHI::ShaderResourceType::ConstantBuffer,
                RHI::ShaderStage::Vertex | RHI::ShaderStage::Fragment | RHI::ShaderStage::Compute
            )
        );

        layerMatricesBuffers.Init("Layer Matrices", 4, RHI::BufferBindFlags::ConstantBuffer);
        drawItemBuffers.Init("Draw Items", 64, RHI::BufferBindFlags::StructuredBuffer);

        auto layerSrgLayout = FApplication::Get()->GetService<FRenderService>()->GetSubPassSrgLayout();

        UpdateLayerSrgs();

        RHI::ShaderResourceGroupDescriptor srgDesc{};
        srgDesc.name = "FSurface SRG_PerView";
        srgDesc.layout = viewSrgLayout;

        viewSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(srgDesc);

        for (u32 i = 0; i < viewConstantBuffers.GetSize(); i++)
        {
            viewSrg->Bind(i, "_PerViewData", viewConstantBuffers[i]);
        }
        
        viewSrg->FlushBindings();
    }

    void FSurface::Shutdown()
    {
        for (auto srg : layerSrgs)
        {
            delete srg;
        }
        layerSrgs.Clear();

        for (auto srg : drawItemSrgs)
        {
            delete srg;
        }
        drawItemSrgs.Clear();

        for (RHI::DrawPacket* drawPacket : drawPackets)
        {
            delete drawPacket;
        }
        drawPackets.Clear();
    }

    void FSurface::QueueDestroy()
    {
        if (Ref<FSurface> parent = parentSurface.Lock())
        {
            parent->RemoveChildSurface(this);
        }
        else
        {
            FApplication::Get()->RemoveSurface(this);
        }

        FApplication::Get()->QueueDestroy(this);
    }

    void FSurface::TickSurface(f32 deltaTime)
    {
		if (!rootWidget)
            return;

        ZoneScoped;

        // - Layout

        try
        {
            HashSet<FWidget*> pendingSet;
            for (auto& root : pendingLayoutRoots)
                pendingSet.Add(root.Get());

            // Remove any root whose ancestor is also pending
            pendingLayoutRoots.RemoveAll([&](Ref<FWidget> root)
                {
                    Ref<FWidget> ancestor = root->GetParentWidget();
                    while (ancestor != nullptr)
                    {
                        if (pendingSet.Exists(ancestor.Get()))
                        {
							pendingLayoutRootIds.Remove(root->GetUuid());
	                        return true;
                        }
                        ancestor = ancestor->GetParentWidget();
                    }
                    return false;
                });

			for (int i = pendingLayoutRoots.GetSize() - 1; i >= 0; i--)
            {
				Ref<FWidget> root = pendingLayoutRoots[i];
				pendingLayoutRoots.RemoveAt(i);
				if (!root)
                    continue;

                pendingLayoutRootIds.Remove(root->GetUuid());

				if (root->IsFaulted())
                    continue;

				Vec2 availableSize = GetAvailableSize();
                if (Ref<FWidget> parentWidget = root->GetParentWidget())
                {
					availableSize.x = Math::Max(0.0f, parentWidget->GetLayoutSize().x - parentWidget->Padding().left - parentWidget->Padding().right);
					availableSize.y = Math::Max(0.0f, parentWidget->GetLayoutSize().y - parentWidget->Padding().top - parentWidget->Padding().bottom);
                }
                
				root->MeasureContent(availableSize);
				root->ArrangeContent(availableSize);
            }
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while calculating Layout.\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        // - Layer Tree Sync

        layerTree->DoSyncIfNeeded(rootWidget.Get());

        // - Paint

        try
        {
            layerTree->DoPaintIfNeeded();
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while painting. Stack Trace:\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        // - Composite


    }

    void FSurface::RenderFrame(u32 frameIndex)
    {
        ZoneScoped;

        Ptr<FRenderSnapshot> renderSnapshot = renderSnapshots[frameIndex];
        
        constexpr RHI::IndexFormat kIndexFormat = sizeof(FUIIndex) == 4 ? RHI::IndexFormat::Uint32 : RHI::IndexFormat::Uint16;

        renderSnapshot->BuildSnapshot(layerTree);

        const u64 vertexArrayByteSize = renderSnapshot->vertexArray.GetCount() * sizeof(FUIVertex);
        const u64 indexArrayByteSize = renderSnapshot->indexArray.GetCount() * sizeof(FUIIndex);

        u64 totalQuadBufferSize = vertexArrayByteSize + indexArrayByteSize;
        u64 currentQuadBufferSize = 0;

        if (totalQuadBufferSize == 0)
            return;

        // - Geometry Buffer -
        
        quadBufferGrowRatio = Math::Clamp(quadBufferGrowRatio, 0.1f, 0.5f);

        if (quadBuffersPerImage[frameIndex] == nullptr || quadBuffersPerImage[frameIndex]->GetBufferSize() < totalQuadBufferSize)
        {
            RHI::Buffer* oldBuffer = quadBuffersPerImage[frameIndex];
            if (oldBuffer)
            {
	            currentQuadBufferSize = oldBuffer->GetBufferSize();
                RPISystem::Get().QueueDestroy(oldBuffer);
                quadBuffersPerImage[frameIndex] = nullptr;
            }

            RHI::BufferDescriptor desc{};
            desc.name = "QuadsBuffer";
            desc.bufferSize = desc.structureByteStride = Math::Max((u64)((f32)currentQuadBufferSize * quadBufferGrowRatio), totalQuadBufferSize);
            desc.defaultHeapType = MemoryHeapType::Upload;
            desc.bindFlags = RHI::BufferBindFlags::VertexBuffer | RHI::BufferBindFlags::IndexBuffer;

            quadBuffersPerImage[frameIndex] = gDynamicRHI->CreateBuffer(desc);
        }

        u8* data = nullptr;
        quadBuffersPerImage[frameIndex]->Map(0, quadBuffersPerImage[frameIndex]->GetBufferSize(), (void**)&data);
        {
            memcpy(data, renderSnapshot->vertexArray.GetData(), vertexArrayByteSize);
            memcpy(data + vertexArrayByteSize, renderSnapshot->indexArray.GetData(), indexArrayByteSize);
        }
        quadBuffersPerImage[frameIndex]->Unmap();

        vertexBufferViewPerImage[frameIndex] = RHI::VertexBufferView(quadBuffersPerImage[frameIndex], 0, vertexArrayByteSize, sizeof(FUIVertex));
        indexBufferViewPerImage[frameIndex] = RHI::IndexBufferView(quadBuffersPerImage[frameIndex], vertexArrayByteSize, indexArrayByteSize, kIndexFormat);

        // - Layer Matrices cbuffer -

        if (layerMatricesBuffers.GetElementCount() < renderSnapshot->matricesPerLayer.GetCount())
        {
            layerMatricesBuffers.GrowToFit(renderSnapshot->matricesPerLayer.GetCount());

            UpdateLayerSrgs();
        }
        
        layerMatricesBuffers.GetBuffer(frameIndex)->Map(0, renderSnapshot->matricesPerLayer.GetByteSize(), (void**)&data);
        {
            memcpy(data, renderSnapshot->matricesPerLayer.GetData(), renderSnapshot->matricesPerLayer.GetByteSize());
        }
        layerMatricesBuffers.GetBuffer(frameIndex)->Unmap();

        // - Draw Items buffer -

        const u64 drawItemCount = renderSnapshot->drawItemArray.GetCount();

        if (drawItemBuffers.GetElementCount() < drawItemCount)
        {
            drawItemBuffers.GrowToFit(drawItemCount);
        }

        if (drawItemCount > 0)
        {
            drawItemBuffers.GetBuffer(frameIndex)->Map(0, drawItemCount * sizeof(FUIDrawItem), (void**)&data);
            {
                memcpy(data, renderSnapshot->drawItemArray.GetData(), drawItemCount * sizeof(FUIDrawItem));
            }
            drawItemBuffers.GetBuffer(frameIndex)->Unmap();
        }

        UpdateDrawItemSrgs(frameIndex);

        // - View Constants -

        viewConstants.pixelResolution = drawableSize.ToVec2();
        viewConstants.projectionMatrix = Matrix4x4::OrthographicProjection(
            0, availableSize.x,
            0, availableSize.y,
            -1.0f, 1.0f
        );
        viewConstants.viewProjectionMatrix = viewConstants.projectionMatrix;
        viewConstants.viewMatrix = Matrix4x4::Identity();
        viewConstants.nearPlane = -1.0f;
        viewConstants.farPlane = 1.0f;

        viewConstantBuffers[frameIndex]->UploadData(&viewConstants, sizeof(viewConstants));
    }

    void FSurface::OnSurfaceResize()
    {
        
    }

    void FSurface::FlushDrawPackets(RHI::DrawListContext& drawList, u32 frameIndex)
    {
        ZoneScoped;

        Ptr<FRenderSnapshot> snapshot = renderSnapshots[frameIndex];

        drawPacketCount = 0;

        for (int i = 0; i < snapshot->renderPassArray.GetCount(); i++)
        {
            const auto& renderPass = snapshot->renderPassArray[i];

            SIZE_T layerIndex = renderPass.layerIndex;

            for (int j = 0; j < renderPass.drawCmdCount; j++)
            {
                int drawCmdIdx = renderPass.drawCmdStartIndex + j;
                FUIDrawCmd drawCmd = snapshot->drawCmdArray[drawCmdIdx];

                u32 globalVertexOffset = snapshot->vertexSplits[layerIndex].startIndex + drawCmd.vertexOffset;
                u32 globalIndexOffset = snapshot->indexSplits[layerIndex].startIndex + drawCmd.indexOffset;

                DrawPacket* packet = nullptr;

                if (drawPackets.GetSize() > drawPacketCount)
                {
                    Ref<FShader> shader = FApplication::Get()->GetService<FRenderService>()->GetMainShader();

                    packet = drawPackets[drawPacketCount];
                    RHI::DrawItem& drawItem = packet->drawItems[0];
                    *drawItem.indexBufferView = indexBufferViewPerImage[frameIndex];
                    drawItem.vertexBufferViews[0] = vertexBufferViewPerImage[frameIndex];

                    packet->shaderResourceGroups[0] = layerSrgs[layerIndex];
                    packet->shaderResourceGroups[1] = viewSrg;
                    packet->shaderResourceGroups[2] = drawItemSrgs[layerIndex];

                    drawItem.pipelineState = shader->GetDefaultPipeline();

                    drawItem.arguments = RHI::DrawIndexedArguments(1, 0, globalVertexOffset, drawCmd.indexCount, globalIndexOffset);

                    drawPacketCount++;
                }
                else
                {
                    Ref<FShader> shader = FApplication::Get()->GetService<FRenderService>()->GetMainShader();

                    RHI::DrawPacketBuilder builder{};
                    builder.AddDrawItem({
                        .stencilRef = 0,
                        .drawItemTag = drawListTag,
                        .indexBufferView = indexBufferViewPerImage[frameIndex],
                        .vertexBufferViews = { vertexBufferViewPerImage[frameIndex] },
                        .uniqueShaderResourceGroups = {},
                        .pipelineState = shader->GetDefaultPipeline(),
                        .drawFilterMask = RHI::DrawFilterMask::ALL
                        });

                    builder.AddShaderResourceGroup(layerSrgs[layerIndex]);
                    builder.AddShaderResourceGroup(viewSrg);
                    builder.AddShaderResourceGroup(drawItemSrgs[layerIndex]);

                    builder.SetDrawArguments(RHI::DrawIndexedArguments(1, 0, globalVertexOffset, drawCmd.indexCount, globalIndexOffset));

                    packet = builder.Build();
                    drawPackets.Add(packet);

                    drawPacketCount++;
                }

                drawList.AddDrawPacket(packet);
            }
        }

        for (Ref<FSurface> childSurface : childrenSurfaces)
        {
            childSurface->FlushDrawPackets(drawList, frameIndex);
        }
    }

    void FSurface::UpdateLayerSrgs()
    {
        ZoneScoped;

        auto layerSrgLayout = FApplication::Get()->GetService<FRenderService>()->GetSubPassSrgLayout();

        for (int i = 0; i < layerMatricesBuffers.GetElementCount(); i++)
        {
            RHI::ShaderResourceGroup* srg = nullptr;

            if (i < layerSrgs.GetSize())
            {
                srg = layerSrgs[i];
            }
            else
            {
                srg = gDynamicRHI->CreateShaderResourceGroup({ "LayerSrg", layerSrgLayout });
                layerSrgs.Add(srg);
            }

            for (int j = 0; j < RHI::Limits::MaxSwapChainImageCount; j++)
            {
                srg->Bind(j, "_LayerData", RHI::BufferView(layerMatricesBuffers.GetBuffer(j), i * sizeof(Matrix4x4), sizeof(Matrix4x4)));
            }

            srg->FlushBindings();
        }
    }

    void FSurface::UpdateDrawItemSrgs(u32 frameIndex)
    {
        ZoneScoped;

        auto objectSrgLayout = FApplication::Get()->GetService<FRenderService>()->GetObjectSrgLayout();

        Ptr<FRenderSnapshot> renderSnapshot = renderSnapshots[frameIndex];

        for (int i = 0; i < (int)renderSnapshot->drawItemSplits.GetCount(); i++)
        {
            RHI::ShaderResourceGroup* srg = nullptr;

            if (i < drawItemSrgs.GetSize())
            {
                srg = drawItemSrgs[i];
            }
            else
            {
                srg = gDynamicRHI->CreateShaderResourceGroup({ "DrawItemSrg", objectSrgLayout });
                drawItemSrgs.Add(srg);
            }

            const auto& split = renderSnapshot->drawItemSplits[i];
            const u64 byteOffset = split.startIndex * sizeof(FUIDrawItem);
            const u64 byteSize = Math::Max(split.count, (SIZE_T)1) * sizeof(FUIDrawItem);

            srg->Bind(frameIndex, "_DrawItems", RHI::BufferView(drawItemBuffers.GetBuffer(frameIndex), byteOffset, byteSize));
            srg->FlushBindings();
        }
    }
} // namespace CE

