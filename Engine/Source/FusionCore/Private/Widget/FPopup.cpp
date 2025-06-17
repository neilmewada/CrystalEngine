#include "FusionCore.h"

namespace CE
{

    FPopup::FPopup()
    {
        m_BlockInteraction = false;
        m_AutoClose = true;
    }

    bool FPopup::FocusParentExistsRecursive(FWidget* parent)
    {
        if (auto parentPopup = this->parentPopup.Lock())
        {
            if (parentPopup.Get() == parent || parentPopup->FocusParentExistsRecursive(parent))
            {
                return true;
            }
        }
        return Super::FocusParentExistsRecursive(parent);
    }

    void FPopup::ClosePopup()
    {
        if (!isShown)
            return;

        Ref<FFusionContext> context = GetContext();
        if (context)
        {
            context->ClosePopup(this);
        }
    }

    void FPopup::OnPopupClosed()
    {
        m_OnClosed(this);
    }

    void FPopup::SetContextWidget(Ref<FWidget> widget)
    {
        this->contextWidget = widget;
    }

    Ref<FWidget> FPopup::GetContextWidget()
    {
        return contextWidget.Lock();
    }

    void FPopup::SetParentPopup(Ref<FPopup> popup)
    {
        this->parentPopup = popup;
    }


    void FPopup::HandleEvent(FEvent* event)
    {
        if (event->type == FEventType::FocusChanged)
        {
            FFocusEvent* focusEvent = static_cast<FFocusEvent*>(event);

            if (focusEvent->GotFocus() != IsFocused())
            {
                if (focusEvent->GotFocus())
                {
					//CE_LOG(Info, All, "Focus Got!");
                }
                else if (!IsOfType<FMenuPopup>())
                {
                    if (AutoClose())
                    {
                        ClosePopup();
                    }
                }
            }
        }
        else if (event->type == FEventType::NativeWindowExposed && m_AutoClose)
        {
            FNativeEvent* nativeEvent = static_cast<FNativeEvent*>(event);
            Ref<FFusionContext> context = GetContext();

            if ((isNativePopup && context != nullptr && context->ParentContextExistsRecursive(nativeEvent->nativeContext))||
                (!isNativePopup && context == nativeEvent->nativeContext))
            {
                ClosePopup();
            }
        }

	    Super::HandleEvent(event);
    }
}

