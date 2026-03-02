#include "FusionCore.h"

namespace CE
{

    FSurface::FSurface()
    {
        compositor = CreateDefaultSubobject<FLayerCompositor>("LayerCompositor");
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

            rootWidget->MarkLayoutDirty();
            rootWidget->MarkPaintDirty();
		}
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

    void FSurface::AddDirtyPaintRoot(Ref<FLayer> paintRoot)
    {
        if (!paintRoot)
            return;

        if (dirtyPaintRootIds.Exists(paintRoot->GetUuid()))
            return;

        paintRoot->MarkPaintDirty();

        dirtyPaintRoots.Add(paintRoot);
        dirtyPaintRootIds.Add(paintRoot->GetUuid());
    }

    void FSurface::TickSurface(f32 deltaTime)
    {
		if (!rootWidget)
            return;

        // - Layout

        try
        {
            HashSet<FWidget*> pendingSet;
            for (auto& root : pendingLayoutRoots)
                pendingSet.Add(root.Get());

            // Remove any root whose ancestor is also pending
            pendingLayoutRoots.RemoveAll([&](const Ref<FWidget>& root)
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

        // - Paint

        try
        {
            HashSet<FLayer*> dirtySet;
            for (auto root : dirtyPaintRoots)
                dirtySet.Add(root.Get());

            // Remove any root whose ancestor is also pending
            dirtyPaintRoots.RemoveAll([&](const Ref<FLayer>& root)
                {
                    Ref<FLayer> ancestor = root->GetParentLayer();
                    while (ancestor != nullptr)
                    {
                        if (dirtySet.Exists(ancestor.Get()))
                        {
                            dirtyPaintRootIds.Remove(root->GetUuid());
	                        return true;
                        }
                        ancestor = ancestor->GetParentLayer();
                    }
                    return false;
                });

			for (int i = dirtyPaintRoots.GetSize() - 1; i >= 0; i--)
            {
                Ref<FLayer> root = dirtyPaintRoots[i];
                dirtyPaintRoots.RemoveAt(i);
                if (!root)
                    continue;

                dirtyPaintRootIds.Remove(root->GetUuid());
                if (root->IsFaulted())
                    continue;

                if (Ref<FWidget> widget = root->GetOwningWidget())
                {
                    if (widget->IsFaulted())
                        continue;

                    root->DoPaintIfNeeded();
                }
            }
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while painting. Stack Trace:\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        // - Composite


    }

} // namespace CE

