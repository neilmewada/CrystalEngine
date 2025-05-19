#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FListWidgetStyle : public FStyledWidgetStyle
    {
        CE_CLASS(FListWidgetStyle, FStyledWidgetStyle)
    public:

        virtual ~FListWidgetStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    protected:

        FListWidgetStyle();

    public:

        FIELD()
        FBrush itemBackground;

        FIELD()
        FBrush selectedItemBackground;

        FIELD()
        FBrush hoveredItemBackground;

        FIELD()
        Color itemBorderColor;

        FIELD()
        Color selectedItemBorderColor;

        FIELD()
        Color hoveredItemBorderColor;

        FIELD()
        f32 itemBorderWidth = 0;

    };
    
} // namespace CE

#include "FListWidgetStyle.rtti.h"
