#include "FusionCore.h"

namespace CE
{

    FContainerWidget::FContainerWidget()
    {

    }

    void FContainerWidget::AddChildWidget(Ref<FWidget> childWidget)
    {
		if (!children.Exists(childWidget))
        {
            children.Add(childWidget);
            childWidget->parentWidget = this;

            MarkLayoutDirty();
        }
    }

    void FContainerWidget::RemoveChildWidget(Ref<FWidget> childWidget)
    {
        if (children.Exists(childWidget))
        {
            children.Remove(childWidget);
            childWidget->parentWidget = nullptr;

            MarkLayoutDirty();
		}
    }

}

