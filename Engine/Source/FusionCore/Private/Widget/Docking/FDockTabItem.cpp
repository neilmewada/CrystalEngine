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

            if (event->type == FEventType::DragBegin)
            {
                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);
            }
            else if (event->type == FEventType::DragMove)
            {
                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);
            }
            else if (event->type == FEventType::DragEnd)
            {
                
            }
        }

        Super::HandleEvent(event);
    }

    bool FDockTabItem::SupportsDragEvents() const
    {
        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
            {

            }
        }

        return false;
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

