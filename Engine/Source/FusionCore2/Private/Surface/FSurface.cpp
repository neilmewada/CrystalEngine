#include "FusionCore.h"

namespace CE
{

    FSurface::FSurface()
    {

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
		this->owningWidget = widget;

		widget->SetParentSurfaceRecursive(this);
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

    void FSurface::AddDirtyPaintRoot(Ref<FWidget> paintRoot)
    {
        if (!paintRoot)
            return;

        if (dirtyPaintRootIds.Exists(paintRoot->GetUuid()))
            return;

        dirtyPaintRoots.Add(paintRoot);
        dirtyPaintRootIds.Add(paintRoot->GetUuid());
    }

    void FSurface::TickSurface(f32 deltaTime)
    {
		if (!owningWidget)
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
                            return true;
                        ancestor = ancestor->GetParentWidget();
                    }
                    return false;
                });

			for (Ref<FWidget> root : pendingLayoutRoots)
            {
                // TODO: Do layout
            }
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while calculating Layout. Stack Trace:\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        pendingLayoutRootIds.Clear();
        pendingLayoutRoots.Clear();

        // - Paint

        try
        {
            HashSet<FWidget*> dirtySet;
            for (auto root : dirtyPaintRoots)
                dirtySet.Add(root.Get());

            // Remove any root whose ancestor is also pending
            dirtyPaintRoots.RemoveAll([&](const Ref<FWidget>& root)
                {
                    Ref<FWidget> ancestor = root->GetParentWidget();
                    while (ancestor != nullptr)
                    {
                        if (dirtySet.Exists(ancestor.Get()))
                            return true;
                        ancestor = ancestor->GetParentWidget();
                    }
                    return false;
                });

            for (Ref<FWidget> root : dirtyPaintRoots)
            {
                // TODO: Do paint
            }
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while painting. Stack Trace:\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        dirtyPaintRootIds.Clear();
        dirtyPaintRoots.Clear();
    }

} // namespace CE

