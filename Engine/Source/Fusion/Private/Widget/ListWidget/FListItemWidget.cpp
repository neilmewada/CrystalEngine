#include "Fusion.h"

namespace CE
{

    FListItemWidget::FListItemWidget()
    {

    }

    void FListItemWidget::Select()
    {
        if (!listView)
            return;

        listView->OnItemSelected(this);
    }

    void FListItemWidget::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && listView && listView->SelectionMode() != FSelectionMode::None)
        {
            auto mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter && !IsHovered())
            {
                itemState |= FListItemState::Hovered;

                if (listView)
                {
	                listView->ApplyStyle();
                }
            }
            else if (mouseEvent->type == FEventType::MouseLeave && IsHovered())
            {
                itemState &= ~FListItemState::Hovered;

                if (listView)
                {
                    listView->ApplyStyle();
                }
            }
            else if (mouseEvent->type == FEventType::MousePress && IsHovered() && !IsSelected() && event->sender == this)
            {
                Select();
            }
        }

	    Super::HandleEvent(event);
    }

}

