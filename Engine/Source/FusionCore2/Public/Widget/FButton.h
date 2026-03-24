#pragma once

namespace CE
{
    class FButton;
    DECLARE_SCRIPT_EVENT(FButtonEvent, FButton*);

    ENUM(Flags)
    enum class FButtonState : u8
    {
	    Default,
        Hovered = BIT(0),
        Pressed = BIT(1),
        InteractionDisabled = BIT(2)
    };
    ENUM_CLASS_FLAGS(FButtonState);

    CLASS()
    class FUSIONCORE_API FButton : public FDecoratedWidget
    {
        CE_CLASS(FButton, FDecoratedWidget)
    protected:

        FButton();

        void Construct() override;

        // - Events -

        void OnMouseEnter(FMouseEvent& event) override;
        void OnMouseLeave(FMouseEvent& event) override;

        FEventReply OnMouseButtonDown(FMouseEvent& event) override;
        FEventReply OnMouseButtonUp(FMouseEvent& event) override;

    public: // - Public API -

        bool IsHovered() const { return EnumHasFlag(buttonState, FButtonState::Hovered); }
        bool IsPressed() const { return EnumHasFlag(buttonState, FButtonState::Pressed); }
        bool IsInteractionDisabled() const { return EnumHasFlag(buttonState, FButtonState::InteractionDisabled); }

    private: // - Internal -

        FIELD(ReadOnly)
        FButtonState buttonState = FButtonState::Default;

    public: // - Fusion Properties - 

        FUSION_EVENT(FButtonEvent, OnClick);

        FUSION_WIDGET;
    };
    
}

#include "FButton.rtti.h"
