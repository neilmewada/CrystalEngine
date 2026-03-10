#include "FusionCore.h"

namespace CE
{

    FContainerWidget::FContainerWidget()
    {

    }

    void FContainerWidget::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        ZoneScoped;

        Super::SetParentSurfaceRecursive(surface);

        for (int i = 0; i < children.GetSize(); i++)
        {
            children[i]->SetParentSurfaceRecursive(surface);
        }
    }

    void FContainerWidget::AddChildWidget(Ref<FWidget> childWidget)
    {
    	ZoneScoped;

        if (!childWidget || children.Exists(childWidget))
            return;

        childWidget->DetachFromParent();

        children.Add(childWidget);

        childWidget->SetParentWidget(this);
        childWidget->SetParentSurfaceRecursive(parentSurface.Lock());
        childWidget->UpdateBoundaryFlags();

        if (Ref<FSurface> surface = GetParentSurface())
        {
            surface->MarkLayerTreeDirty();
        }

        MarkLayoutDirty();
        MarkPaintDirty();
    }

    void FContainerWidget::RemoveChildWidget(Ref<FWidget> childWidget)
    {
        DetachChild(childWidget);
    }

    void FContainerWidget::DetachChild(Ref<FWidget> child)
    {
        ZoneScoped;

        Super::DetachChild(child);

        if (const int index = (int)children.IndexOf(child); index >= 0)
        {
            if (Ref<FSurface> surface = GetParentSurface())
            {
                surface->MarkLayerTreeDirty();
            }

            children.RemoveAt(index);
            child->SetParentWidget(nullptr);
            child->SetParentSurfaceRecursive(nullptr);

            MarkLayoutDirty();
            MarkPaintDirty();
        }
    }
}

