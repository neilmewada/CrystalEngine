#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FListViewStyle : public FStyledWidgetStyle
    {
        CE_CLASS(FListViewStyle, FStyledWidgetStyle)
    public:

        virtual ~FListViewStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    protected:

        FListViewStyle();

    public:

        FIELD()
        FBrush background = Color::RGBA(26, 26, 26);

        FIELD()
        FBrush itemBackground = Color::RGBA(26, 26, 26);

        FIELD()
        FBrush alternateItemBackground = Color::RGBA(21, 21, 21);

        FIELD()
        FBrush selectedItemBackground = Color::RGBA(0, 112, 224);

        FIELD()
        FBrush hoveredItemBackground = Color::RGBA(36 ,36 , 36);

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

#include "FListViewStyle.rtti.h"
