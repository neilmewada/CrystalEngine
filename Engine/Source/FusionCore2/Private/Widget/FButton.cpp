#include "FusionCore.h"

namespace CE
{

    FButton::FButton()
    {
        m_Style = "Button/Default";
    }

    void FButton::Construct()
    {
        Super::Construct();

        
    }

    FEventReply FButton::HandleEvent(FEvent& event)
    {
        if (event.IsMouseEvent())
        {
            FMouseEvent& mouseEvent = static_cast<FMouseEvent&>(event);

            if (event.eventType == FEventType::MouseEnter)
            {
	            if (!IsHovered())
	            {
                    buttonState |= FButtonState::Hovered;
                    NotifyStyleStateChanged();
	            }
                return FEventReply::Handled();
            }
            else if (event.eventType == FEventType::MouseLeave)
            {
                if (IsHovered())
                {
                    buttonState &= ~FButtonState::Hovered;
                    NotifyStyleStateChanged();
                }
                return FEventReply::Handled();
            }
            else if (event.eventType == FEventType::MouseMove)
            {
	            
            }
            else if (event.eventType == FEventType::MouseButtonDown && mouseEvent.IsLeftButton())
            {
	            
            }
            else if (event.eventType == FEventType::MouseButtonUp && mouseEvent.IsLeftButton())
            {
                if (mouseEvent.isInside)
                {
                    m_OnClick.Broadcast(this);
                }

                return FEventReply::Handled();
            }
        }

	    return Super::HandleEvent(event);
    }

}

