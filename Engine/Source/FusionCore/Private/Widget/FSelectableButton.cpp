#include "FusionCore.h"

namespace CE
{

    FSelectableButton::FSelectableButton()
    {

    }

    void FSelectableButton::Construct()
    {
        Super::Construct();
        
    }

    void FSelectableButton::Select()
    {
        if (!EnumHasFlag(buttonState, FSelectableButtonState::Active))
        {
            buttonState |= FSelectableButtonState::Active;
            ApplyStyle();
        }
    }

    void FSelectableButton::Deselect()
    {
        if (EnumHasFlag(buttonState, FSelectableButtonState::Active))
        {
            buttonState &= ~FSelectableButtonState::Active;
            ApplyStyle();
        }
    }

    void FSelectableButton::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && !IsInteractionDisabled() && IsVisible())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter && event->sender == this)
            {
	            if (!EnumHasFlag(buttonState, FSelectableButtonState::Hovered))
	            {
                    buttonState |= FSelectableButtonState::Hovered;
                    ApplyStyle();
	            }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MouseLeave && event->sender == this)
            {
	            if (EnumHasFlag(buttonState, FSelectableButtonState::Hovered))
	            {
                    buttonState &= ~FSelectableButtonState::Hovered;
                    ApplyStyle();
	            }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left && !mouseEvent->isConsumed)
            {
	            if (!EnumHasFlag(buttonState, FSelectableButtonState::Active))
	            {
                    buttonState |= FSelectableButtonState::Active;
                    ApplyStyle();

	                m_OnSelect(this);
	            }
                if (mouseEvent->isDoubleClick)
                {
                    m_OnDoubleClick(this);
                }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Right && !mouseEvent->isConsumed &&
                !mouseEvent->isDoubleClick)
            {
                m_OnRightClick(this);
            }
            else if (mouseEvent->type == FEventType::MouseRelease && mouseEvent->buttons == MouseButtonMask::Left)
            {
                event->Consume(this);
            }
        }

        Super::HandleEvent(event);
    }

}

