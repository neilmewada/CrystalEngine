#include "FusionCore.h"

namespace CE
{

    FTabItem::FTabItem()
    {
        m_Padding = Vec4(1, 1, 1, 1) * 2.5f;
    }

    void FTabItem::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && tabView)
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter && event->sender == this)
            {
                itemFlags |= FTabItemFlags::Hovered;
                tabView->ApplyStyle();
            }
            else if (mouseEvent->type == FEventType::MouseLeave && event->sender == this)
            {
                itemFlags &= ~FTabItemFlags::Hovered;
                tabView->ApplyStyle();
            }
            else if (mouseEvent->type == FEventType::MousePress)
            {
                tabView->SelectTab(this);
            }
        }

	    Super::HandleEvent(event);
    }

    FLabelTabItem::FLabelTabItem()
    {
        
    }

    void FLabelTabItem::Construct()
    {
	    Super::Construct();

        Child(
            FAssignNew(FLabel, label)
            .FontSize(14)
            .Foreground(Colors::White)
        );
    }

}

