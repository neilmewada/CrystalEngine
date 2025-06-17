#include "FusionCore.h"

namespace CE
{

    FDockTabItem::FDockTabItem()
    {

    }

    void FDockTabItem::Construct()
    {
        Super::Construct();

        Child(
            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            .Padding(Vec4(2, 1, 2, 1) * 7.5f)
            (
                FAssignNew(FLabel, tabTitle)
                .FontSize(11)
            )
        );
    }

    void FDockTabItem::HandleEvent(FEvent* event)
    {
        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (event->IsMouseEvent())
            {
                FMouseEvent* mouseEvent = (FMouseEvent*)event;

                if (event->type == FEventType::MouseEnter)
                {
                    isHovered = true;
                    ApplyStyle();
                }
                else if (event->type == FEventType::MouseLeave)
                {
                    isHovered = false;
                    ApplyStyle();
                }
                else if (event->type == FEventType::MousePress)
                {
                    SetActiveTab();
                }
            }

            if (event->IsDragEvent())
            {
                FDragEvent* dragEvent = (FDragEvent*)event;

                if (dragEvent->type == FEventType::DragMove && shouldDetach)
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    shouldDetach = false;
                    isOutside = true;
                }
                else if (dragEvent->type == FEventType::DragEnd && isOutside)
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    shouldDetach = false;
                    isOutside = false;
                }
            }
        }

        Super::HandleEvent(event);
    }

    bool FDockTabItem::CanBeDetached()
    {
        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
            {
                return dockspace->CanDetach(this);
            }
        }

        return false;
    }

    bool FDockTabItem::DetachItem()
    {
        if (!CanBeDetached())
            return false;

        shouldDetach = false;

        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
            {
                if (Ref<FNativeContext> detachedWindow = dockspace->DetachItem(this))
                {


                    shouldDetach = true;
                }
            }
        }

        return shouldDetach;
    }

    bool FDockTabItem::SupportsDragEvents() const
    {
        return true;
    }

    void FDockTabItem::SetActiveTab()
    {
        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            tabWell->SetActiveTab(this);
        }
    }

    void FDockTabItem::ApplyStyle()
    {
        Super::ApplyStyle();

        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            tabWell->ApplyStyle();
        }
    }
}

