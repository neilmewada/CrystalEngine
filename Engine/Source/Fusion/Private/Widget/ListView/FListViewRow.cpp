#include "Fusion.h"

namespace CE
{

    FListViewRow::FListViewRow()
    {

    }

    void FListViewRow::Construct()
    {
        Super::Construct();


    }

    void FListViewRow::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            FMouseEvent* mouseEvent = (FMouseEvent*)event;

            if (mouseEvent->type == FEventType::MouseEnter)
            {
                isHovered = true;
                if (auto owner = listView.Lock())
                {
                    owner->ApplyStyle();
                }
            }
            else if (mouseEvent->type == FEventType::MouseLeave)
            {
                isHovered = false;
                if (auto owner = listView.Lock())
                {
                    owner->ApplyStyle();
                }
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left && mouseEvent->sender == this)
            {
                if (auto owner = listView.Lock())
                {
                    owner->SelectRow(this, EnumHasAnyFlags(mouseEvent->keyModifiers, KeyModifier::Ctrl | KeyModifier::Shift));
                }
            }
        }

        Super::HandleEvent(event);
    }
}

