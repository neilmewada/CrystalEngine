#include "FusionCore.h"

namespace CE
{
    
    FToolWindow::FToolWindow()
    {
	    
    }

    void FToolWindow::SetMaximizeButton(bool interactable)
    {
        Color tintColor = interactable ? Colors::White : Color::RGBA(255, 255, 255, 80);

        //maximizeIcon->Background(maximizeIcon->Background().WithTint(tintColor));
        maximizeButton->SetInteractionEnabled(interactable);
    }

    void FToolWindow::SetMinimizeButton(bool interactable)
    {
        Color tintColor = interactable ? Colors::White : Color::RGBA(255, 255, 255, 80);

        //minimizeIcon->Background(minimizeIcon->Background().WithTint(tintColor));
        minimizeButton->SetInteractionEnabled(interactable);
    }

    void FToolWindow::SetWindowContent(FWidget& contentWidget)
    {
        this->content->RemoveAllChildren();
        this->content->AddChild(contentWidget);
    }

    void FToolWindow::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FStyledWidget, borderWidget)
            .Background(FBrush(Color::RGBA(36, 36, 36)))
            .BorderWidth(1.0f)
            .BorderColor(Color::RGBA(15, 15, 15))
            .Padding(Vec4(1, 1, 1, 1) * 2)
            .Name("RootBorderWidget")
            (
                FAssignNew(FVerticalStack, rootBox)
                .ContentHAlign(HAlign::Fill)
                .ContentVAlign(VAlign::Top)
                .Name("RootBox")
                (
                    // - Title Bar Begin -

                    FAssignNew(FTitleBar, titleBar)
                    .Background(FBrush(Color::RGBA(26, 26, 26)))
                    .Height(35)
                    .HAlign(HAlign::Fill)
                    (
                        FNew(FOverlayStack)
                        .VAlign(VAlign::Fill)
                        .HAlign(HAlign::Fill)
                        (
                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            (
                                FNew(FWidget)
                                .FillRatio(1.0f),

                                FAssignNew(FLabel, titleBarLabel)
                                .FontSize(12)
                                .Text("Tool Window")
                                .HAlign(HAlign::Center)
                                .VAlign(VAlign::Center),

                                FNew(FWidget)
                                .FillRatio(1.0f)
                            ),

                            FAssignNew(FHorizontalStack, controlContainer)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            (
                                FNew(FWidget)
                                .FillRatio(1.0f),

                                FAssignNew(FWindowControlButton, minimizeButton)
                                .ControlType(FWindowControlType::Minimize)
                                .OnClicked([this]
                                    {
                                        CastTo<FNativeContext>(GetContext())->Minimize();
                                    })
                                .Name("WindowMinimizeButton")
                                .Style("Button.WindowControl"),

                                FAssignNew(FWindowControlButton, maximizeButton)
                                .ControlType(FWindowControlType::Maximize)
                                .OnClicked([this]
                                    {
                                        Ref<FNativeContext> nativeContext = CastTo<FNativeContext>(GetContext());
                                        if (nativeContext->IsMaximized())
                                        {
                                            nativeContext->Restore();
                                        }
                                        else
                                        {
                                            nativeContext->Maximize();
                                        }
                                    })
                                .Name("WindowMaximizeButton")
                                .Style("Button.WindowControl"),

                                FAssignNew(FWindowControlButton, closeButton)
                                .ControlType(FWindowControlType::Close)
                                .OnClicked([this]
                                    {
                                        OnClickClose();
                                        GetContext()->QueueDestroy();
                                    })
                                .Name("WindowCloseButton")
                                .Style("Button.WindowClose")
                            )
                        )
                    ),

                    // - Title Bar End -

                    // - Body Begin -

                    FAssignNew(FStackBox, content)
                    .Direction(FStackBoxDirection::Vertical)
                    .Padding(Vec4(10, 10, 10, 10))
                    .VAlign(VAlign::Fill)
                    .FillRatio(1.0f)
                    .Name("ContentVStack")

                    // - Body End -
                )
            )
        );

        Array<WeakRef<FWindowControlButton>> controlGroup = {
            closeButton,
            minimizeButton,
            maximizeButton
        };

        closeButton->SetControlGroup(controlGroup);
        minimizeButton->SetControlGroup(controlGroup);
        maximizeButton->SetControlGroup(controlGroup);

        if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
        {
            controlContainer->MoveChildToIndex(closeButton, 0);
            controlContainer->MoveChildToIndex(minimizeButton, 1);
            controlContainer->MoveChildToIndex(maximizeButton, 2);

            closeButton->Margin(Vec4(5, 0, -1, 0));
            minimizeButton->Margin(Vec4(0, 0, -1, 0));
        }

        this->Style("ToolWindow");
    }

    void FToolWindow::OnMaximized()
    {
	    Super::OnMaximized();

        maximizeButton->SetMaximizedState(true);

#if PLATFORM_WINDOWS
        // This is needed on Windows to prevent things from rendering outside the screen edges when maximized
        borderWidget->Padding(Vec4(1, 1, 1, 1) * 7);
#endif
    }

    void FToolWindow::OnMinimized()
    {
        Super::OnMinimized();
    }

    void FToolWindow::OnRestored()
    {
        Super::OnRestored();

        maximizeButton->SetMaximizedState(false);

#if PLATFORM_WINDOWS
        borderWidget->Padding(Vec4(1, 1, 1, 1) * 1);
#endif
    }

    FToolWindow::Self& FToolWindow::MinimizeEnabled(bool enabled)
    {
        minimizeButton->Enabled(enabled);
        return *this;
    }

    FToolWindow::Self& FToolWindow::MinimizeInteractable(bool interactable)
    {
        SetMinimizeButton(interactable);
        return *this;
    }

    FToolWindow::Self& FToolWindow::MaximizeEnabled(bool enabled)
    {
        maximizeButton->Enabled(enabled);
        return *this;
    }

    FToolWindow::Self& FToolWindow::MaximizeInteractable(bool interactable)
    {
        SetMaximizeButton(interactable);
        return *this;
    }

} // namespace CE
