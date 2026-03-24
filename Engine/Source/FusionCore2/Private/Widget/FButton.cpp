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

    void FButton::OnMouseEnter(FMouseEvent& event)
    {
        Super::OnMouseEnter(event);

        if (!IsHovered())
        {
            buttonState |= FButtonState::Hovered;
            NotifyStyleStateChanged();
        }
    }

    void FButton::OnMouseLeave(FMouseEvent& event)
    {
	    Super::OnMouseLeave(event);

        if (IsHovered())
        {
            buttonState &= ~FButtonState::Hovered;
            NotifyStyleStateChanged();
        }
    }

    FEventReply FButton::OnMouseButtonDown(FMouseEvent& event)
    {
        if (event.IsLeftButton())
        {
	        buttonState |= FButtonState::Pressed; 
        	NotifyStyleStateChanged();
        }
        return FEventReply::Handled();
    }

    FEventReply FButton::OnMouseButtonUp(FMouseEvent& event)
    {
        if (event.IsLeftButton())
        {
            if (IsPressed())
            {
                buttonState &= ~FButtonState::Pressed;
                NotifyStyleStateChanged();
            }
            if (event.isInside)
            {
                m_OnClick.Broadcast(this);
            }
        }

        return FEventReply::Handled();
    }


}

