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
		if (!children.Exists(childWidget))
        {
		    childWidget->DetachFromParent();

            children.Add(childWidget);

		    childWidget->SetParentWidget(this);
		    childWidget->SetParentSurfaceRecursive(parentSurface.Lock());

            MarkLayoutDirty();
        }
    }

    void FContainerWidget::RemoveChildWidget(Ref<FWidget> childWidget)
    {
        DetachChild(childWidget);
    }

    void FContainerWidget::DetachChild(Ref<FWidget> child)
    {
        ZoneScoped;

        Super::DetachChild(child);

        if (const int index = children.IndexOf(child); index >= 0)
        {
            children.RemoveAt(index);
            child->SetParentWidget(nullptr);
            child->SetParentSurfaceRecursive(nullptr);

            MarkLayoutDirty();
        }
    }
}

