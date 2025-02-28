#pragma once

namespace CE
{
    class FSelectableButton;
    DECLARE_SCRIPT_EVENT(FSelectableButtonEvent, FSelectableButton*);

    ENUM(Flags)
    enum class FSelectableButtonState : u8
    {
        Default = 0,
        Hovered = BIT(0),
        Active = BIT(1),
        InteractionDisabled = BIT(2)
    };
    ENUM_CLASS_FLAGS(FSelectableButtonState);

    CLASS()
    class FUSIONCORE_API FSelectableButton : public FStyledWidget
    {
        CE_CLASS(FSelectableButton, FStyledWidget)
    protected:

        FSelectableButton();

        void Construct() override;

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsFocusEvents() const override { return true; }

        bool IsHovered() const { return EnumHasFlag(buttonState, FSelectableButtonState::Hovered); }
        bool IsActive() const { return EnumHasFlag(buttonState, FSelectableButtonState::Active); }
        bool IsInteractionDisabled() const { return EnumHasFlag(buttonState, FSelectableButtonState::InteractionDisabled); }

        void Select();
        void Deselect();

    protected: // - Internal -

        void HandleEvent(FEvent* event) override;

        FIELD()
        FSelectableButtonState buttonState = FSelectableButtonState::Default;

    public: // - Fusion Properties -

        FUSION_EVENT(FSelectableButtonEvent, OnSelect);
        FUSION_EVENT(FSelectableButtonEvent, OnDoubleClick);
        FUSION_EVENT(FSelectableButtonEvent, OnRightClick);

        FUSION_WIDGET;
    };
    
}

#include "FSelectableButton.rtti.h"
