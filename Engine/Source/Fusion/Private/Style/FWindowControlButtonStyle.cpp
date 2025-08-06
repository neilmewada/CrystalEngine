#include "Fusion.h"

namespace CE
{

    FWindowControlButtonStyle::FWindowControlButtonStyle()
    {

    }

    FWindowControlButtonStyle::~FWindowControlButtonStyle()
    {
        
    }

    SubClass<FWidget> FWindowControlButtonStyle::GetWidgetClass() const
    {
        return FWindowControlButton::StaticClass();
    }

    void FWindowControlButtonStyle::MakeStyle(FWidget& widget)
    {
        Super::MakeStyle(widget);

        FWindowControlButton& button = widget.As<FWindowControlButton>();

        if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
        {
            Color bg = Colors::Clear;

            bool shouldShowIcon = false;
            for (const auto& controlBtn : button.GetControlGroup())
            {
                if (controlBtn->IsPressed() || controlBtn->IsHovered())
                {
                    shouldShowIcon = true;
                    break;
                }
            }

            switch (button.ControlType())
            {
            case FWindowControlType::Undefined:
                break;
            case FWindowControlType::Close:
                bg = Color::RGBHex(0xFF5F58);
                if (button.IsPressed())
                    bg = Color::RGBHex(0xFF8D85);
                break;
            case FWindowControlType::Maximize:
                bg = Color::RGBHex(0x29C740);
                if (button.IsPressed())
                    bg = Color::RGBHex(0x5AF770);
                break;
            case FWindowControlType::Minimize:
                bg = Color::RGBHex(0xFEBC2E);
                if (button.IsPressed())
                    bg = Color::RGBHex(0xFEEA5C);
                break;
            }

            if (button.IsInteractionDisabled())
            {
                bg = Color::RGBA(90, 90, 90);
            }

            button.IconVisible(shouldShowIcon && !button.IsInteractionDisabled());

            button
            .Background(bg)
            .BackgroundShape(FCircle())
            .Padding(Vec4(18, 8, 18, 8) / 2.5f)
            .VAlign(VAlign::Center);
        }
        else // Windows or Linux
        {
            button
            .Padding(Vec4(18, 8, 18, 8))
            .VAlign(VAlign::Top);
        }
    }
} // namespace CE

