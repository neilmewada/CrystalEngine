#pragma once


namespace CE
{
    class FMenuPopup;

    ENUM(Flags)
    enum class FButtonState : u8
    {
	    Default = 0,
        Hovered = BIT(0),
        Pressed = BIT(1),
        InteractionDisabled = BIT(2)
    };
    ENUM_CLASS_FLAGS(FButtonState);

    CLASS()
    class FUSIONCORE_API FButton : public FStyledWidget
    {
        CE_CLASS(FButton, FStyledWidget)
    public:

        FButton();

        FButtonState GetButtonState() const { return buttonState; }

        bool IsHovered() const { return EnumHasFlag(buttonState, FButtonState::Hovered); }
        bool IsPressed() const { return EnumHasFlag(buttonState, FButtonState::Pressed); }
        bool IsInteractionDisabled() const { return EnumHasFlag(buttonState, FButtonState::InteractionDisabled); }

        void SetInteractionEnabled(bool enabled);

        void HandleEvent(FEvent* event) override;

        void OnPaintContentOverlay(FPainter* painter) override;

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsKeyboardEvents() const override { return true; }

        bool SupportsFocusEvents() const override { return true; }

    protected: // - Internal Methods -

        void SetState(FButtonState newState);

        virtual void OnClick() {}
        virtual void OnDoubleClick() {}

    protected: // - Fields -

        FIELD(NonSerialized)
        FButtonState buttonState = FButtonState::Default;

    private:

        bool cursorPushed = false;
        Ref<FMenuPopup> dropDownMenu;

    public: // - Fusion Properties -

        FUSION_EVENT(FVoidEvent, OnClicked);
        FUSION_EVENT(FVoidEvent, OnDoubleClicked);

        FUSION_EVENT(ScriptEvent<void(FButton*, Vec2)>, OnButtonClicked);

        FUSION_PROPERTY(SystemCursor, Cursor);

        Self& DropDownMenu(FMenuPopup& dropDownMenu);

        bool Interactable() const { return !IsInteractionDisabled(); }

        Self& Interactable(bool interactable)
        {
            SetInteractionEnabled(interactable);
	        return *this;
        }

        FUSION_TESTS;
        FUSION_WIDGET;
    };
    
}

#include "FButton.rtti.h"
