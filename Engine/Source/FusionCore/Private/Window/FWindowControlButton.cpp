#include "FusionCore.h"

namespace CE
{

    FWindowControlButton::FWindowControlButton()
    {

    }

    void FWindowControlButton::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FImage, icon)
            .Background(FBrush("/Engine/Resources/Icons/MaximizeIcon"))
            .Width(9)
            .Height(9)
            .HAlign(HAlign::Center)
            .VAlign(VAlign::Center)
        );
    }

    void FWindowControlButton::SetMaximizedState(bool isMaximized)
    {
        if (controlType == FWindowControlType::Maximize && PlatformMisc::GetCurrentPlatform() != PlatformName::Mac)
        {
            icon->Background(FBrush(isMaximized ? "/Engine/Resources/Icons/RestoreIcon" : "/Engine/Resources/Icons/MaximizeIcon"));
        }
    }

    FWindowControlButton::Self& FWindowControlButton::ControlType(FWindowControlType type)
    {
        this->controlType = type;

        Ref<FFusionContext> ctx = GetContext();

        FBrush iconImage = FBrush();
        switch (type)
        {
        case FWindowControlType::Undefined:
            break;
        case FWindowControlType::Close:
            iconImage = FBrush("/Engine/Resources/Icons/CrossIcon");
            break;
        case FWindowControlType::Maximize:
            iconImage = FBrush("/Engine/Resources/Icons/MaximizeIcon");
            if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
                iconImage = FBrush("/Engine/Resources/Icons/PlusIcon");
            break;
        case FWindowControlType::Minimize:
            iconImage = FBrush("/Engine/Resources/Icons/MinimizeIcon");
            break;
        }

        if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
        {
            icon->Width(5);
            icon->Height(5);
            iconImage.SetBrushSize(Vec2(6, 6));
            iconImage.SetFillColor(Color::Black());
        }

        icon->Background(iconImage);

        return *this;
    }

    void FWindowControlButton::HandleEvent(FEvent* event)
    {
        Super::HandleEvent(event);

        if (event->IsMouseEvent() && !IsInteractionDisabled() && IsVisible())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter && mouseEvent->sender == this)
            {
                for (const auto& controlBtn : controlGroup)
                {
                    if (controlBtn != this)
                    {
                        controlBtn->ApplyStyle();
                    }
                }
            }
            else if (mouseEvent->type == FEventType::MouseLeave && event->sender == this)
            {
                for (const auto& controlBtn : controlGroup)
                {
                    if (controlBtn != this)
                    {
                        controlBtn->ApplyStyle();
                    }
                }
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left)
            {
                for (const auto& controlBtn : controlGroup)
                {
                    if (controlBtn != this)
                    {
                        controlBtn->ApplyStyle();
                    }
                }
            }
            else if (mouseEvent->type == FEventType::MouseRelease && mouseEvent->buttons == MouseButtonMask::Left)
            {
                for (const auto& controlBtn : controlGroup)
                {
                    if (controlBtn != this)
                    {
                        controlBtn->ApplyStyle();
                    }
                }
            }
        }
    }

}

