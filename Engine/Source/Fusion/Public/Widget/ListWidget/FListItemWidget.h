#pragma once

namespace CE
{
    ENUM(Flags)
    enum class FListItemState
    {
	    None = 0,
        Hovered = BIT(0),
        Selected = BIT(1)
    };
    ENUM_CLASS_FLAGS(FListItemState);

    CLASS()
    class FUSION_API FListItemWidget : public FStyledWidget
    {
        CE_CLASS(FListItemWidget, FStyledWidget)
    public:

        FListItemWidget();

        // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool IsHovered() const { return EnumHasFlag(itemState, FListItemState::Hovered); }

        bool IsSelected() const { return EnumHasFlag(itemState, FListItemState::Selected); }

        void Select();

    protected:

        void HandleEvent(FEvent* event) override;

        FListWidget* listView = nullptr;

        FListItemState itemState = FListItemState::None;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FListItemStyle;
        friend class FListWidget;
    };
    
}

#include "FListItemWidget.rtti.h"
