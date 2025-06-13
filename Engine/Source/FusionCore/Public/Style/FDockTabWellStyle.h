#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabWellStyle : public FStyle
    {
        CE_CLASS(FDockTabWellStyle, FStyle)
    protected:

        FDockTabWellStyle();
        
    public:

        virtual ~FDockTabWellStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

        FIELD()
        FBrush background = Color::RGBA(26, 26, 26);

        FIELD()
        Color borderColor;

        FIELD()
        f32 borderWidth = 0;

        FIELD()
        FBrush tabBackground = Color::RGBA(26, 26, 26, 0);

        FIELD()
        FBrush tabHoverBackground = Color::RGBA(30, 30, 30);

        FIELD()
        FBrush tabActiveBackground = Color::RGBA(36, 36, 36);

        FIELD()
        Vec4 tabCornerRadius = Vec4(5, 5, 0, 0);

        FIELD()
        Color tabBorderColor;

        FIELD()
        Color tabHoverBorderColor;

        FIELD()
        Color tabActiveBorderColor;

        FIELD()
        f32 tabBorderWidth = 0;
    };
    
} // namespace CE

#include "FDockTabWellStyle.rtti.h"
