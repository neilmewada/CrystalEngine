#include "FusionCore.h"

namespace CE
{

    FSelectableButtonStyle::FSelectableButtonStyle()
    {
        borderWidth = 1.0f;
        cornerRadius = Vec4(1, 1, 1, 1) * 2.5f;

        selectedBackground = Color::RGBHex(0x0270DF);
        selectedBorderColor = Color::RGBHex(0x0270DF);

        background = Color::RGBA(56, 56, 56);
        disabledBackground = Color::RGBA(46, 46, 46);
        hoveredBackground = Color::RGBA(62, 62, 62);
    }

    FSelectableButtonStyle::~FSelectableButtonStyle()
    {
        
    }

    SubClass<FWidget> FSelectableButtonStyle::GetWidgetClass() const
    {
        return FSelectableButton::StaticClass();
    }

    void FSelectableButtonStyle::MakeStyle(FWidget& widget)
    {
        FSelectableButton& self = widget.As<FSelectableButton>();

        FBrush bg = background;
        Color border = borderColor;

        if (self.IsActive())
        {
            bg = selectedBackground;
            border = selectedBorderColor;
        }
        else if (self.IsHovered())
        {
            bg = hoveredBackground;
            border = hoveredBorderColor;
        }

        FShape shape = backgroundShape;
        if (backgroundShape == FShapeType::RoundedRect)
        {
            shape.SetCornerRadius(cornerRadius);
        }

        self
            .Background(bg)
            .Border(border, borderWidth)
            .BackgroundShape(shape)
            .CornerRadius(cornerRadius)
            ;
    }
} // namespace CE

