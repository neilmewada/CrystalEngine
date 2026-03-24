#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FButtonStyle : public FStyle
    {
        CE_CLASS(FButtonStyle, FStyle)
    protected:

        FButtonStyle();
        
    public:

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) const override;

        FIELD()
        FBrush backgroundDefault;

        FIELD()
        FBrush backgroundHovered;

        FIELD()
        FBrush backgroundPressed;

        FIELD()
        FBrush backgroundDisabled;

        FIELD()
        FPen borderDefault;

        FIELD()
        FPen borderHovered;

        FIELD()
        FPen borderPressed;

        FIELD()
        FPen borderDisabled;

        FIELD()
        FShape shape = FRoundedRectangle(2.0f);

        FIELD()
        f32 opacityDisabled = 1.0f;

        FIELD()
        f32 scaleHovered = 1.0f;

        FIELD()
        f32 scalePressed = 1.0f;

        FIELD()
        Vec2 pressedTranslation = Vec2(0, 2.5f);
    };
    
} // namespace CE

#include "FButtonStyle.rtti.h"
