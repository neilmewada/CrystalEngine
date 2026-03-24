#include "FusionCore.h"

namespace CE
{

    FButtonStyle::FButtonStyle()
    {
        
    }

    SubClass<FWidget> FButtonStyle::GetWidgetClass() const
    {
	    return FButton::StaticClass();
    }

    void FButtonStyle::MakeStyle(FWidget& widget) const
    {
        FButton& button = widget.As<FButton>();

        const bool isDisabled = button.IsInteractionDisabled();
        const bool isPressed = !isDisabled && button.IsPressed() && button.IsHovered();
        const bool isHovered = !isDisabled && button.IsHovered();

        // --- Properties ---

        const FBrush bg =
            isDisabled ? backgroundDisabled :
            isPressed ? backgroundPressed :
            isHovered ? backgroundHovered : backgroundDefault;

        const FPen border =
            isDisabled ? borderDisabled :
            isPressed ? borderPressed :
            isHovered ? borderHovered : borderDefault;

        const f32 scale = isPressed ? scalePressed : isHovered ? scaleHovered : 1.0f;

        button
            .Background(bg)
            .Border(border)
            .Shape(shape)
            .Transform(FAffineTransform::Scale(Vec2(scale, scale)));

        if (Ref<FWidget> child = button.GetChild())
        {
            child->Transform(FAffineTransform::Translation((button.IsPressed() && button.IsHovered()) ? pressedTranslation : Vec2(0, 0)));
        }
    }
} // namespace CE

