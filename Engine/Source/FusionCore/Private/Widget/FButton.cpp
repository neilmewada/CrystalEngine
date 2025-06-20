#include "FusionCore.h"

namespace CE
{

    FButton::FButton()
    {
        m_Cursor = SystemCursor::Default;
    }

    void FButton::SetInteractionEnabled(bool enabled)
    {
        if (IsInteractionDisabled() == !enabled)
            return;

        if (enabled)
        {
            buttonState &= ~FButtonState::InteractionDisabled;
        }
        else
        {
            if (cursorPushed)
            {
                cursorPushed = false;
                FusionApplication::Get()->PopCursor();
            }

            buttonState = FButtonState::InteractionDisabled;
        }

        ApplyStyle();
    }

    void FButton::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && !IsInteractionDisabled() && IsVisible())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter && event->sender == this)
            {
	            if (!EnumHasFlag(buttonState, FButtonState::Hovered))
	            {
                    buttonState |= FButtonState::Hovered;
                    ApplyStyle();

                    if (!cursorPushed)
                    {
                        cursorPushed = true;
                        FusionApplication::Get()->PushCursor(m_Cursor);
                    }
	            }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MouseLeave && event->sender == this)
            {
	            if (EnumHasFlag(buttonState, FButtonState::Hovered))
	            {
                    buttonState &= ~FButtonState::Hovered;
                    ApplyStyle();

                    if (cursorPushed)
                    {
                        cursorPushed = false;
                        FusionApplication::Get()->PopCursor();
                    }
	            }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left && !mouseEvent->isConsumed)
            {
	            if (!EnumHasFlag(buttonState, FButtonState::Pressed))
	            {
                    buttonState |= FButtonState::Pressed;
                    ApplyStyle();

	                if (dropDownMenu.IsValid())
	                {
	                    Vec2 popupPos = GetGlobalPosition() + Vec2(0, GetComputedSize().height);
	                    dropDownMenu->MinWidth(GetComputedSize().width);
	                    if (FPopup* curPopup = FindPopupInParent())
	                    {
	                        dropDownMenu->SetParentPopup(curPopup);
	                    }

	                    GetContext()->PushLocalPopup(dropDownMenu.Get(), popupPos, Vec2(), GetComputedSize());
	                }
	            }
                if (mouseEvent->isDoubleClick)
                {
                    OnDoubleClick();
                    m_OnDoubleClicked();
                }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MouseRelease && mouseEvent->buttons == MouseButtonMask::Left)
            {
                if (EnumHasFlag(buttonState, FButtonState::Pressed))
                {
                    buttonState &= ~FButtonState::Pressed;
                    ApplyStyle();

                    if (mouseEvent->isInside && !mouseEvent->isDoubleClick)
                    {
                        OnClick();
                        m_OnClicked();

                        m_OnButtonClicked(this, mouseEvent->mousePosition);
                    }
                }
                event->Consume(this);
            }
        }

	    Super::HandleEvent(event);
    }

    void FButton::SetState(FButtonState newState)
    {
        if (buttonState == newState)
            return;

        ZoneScoped;

        buttonState = newState;

        if (m_Style && !IsDefaultInstance())
        {
            m_Style->MakeStyle(*this);
        }
    }

    FButton::Self& FButton::DropDownMenu(FMenuPopup& dropDownMenu)
    {
        this->dropDownMenu = &dropDownMenu;

        return *this;
    }
}

