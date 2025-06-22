#include "FusionCore.h"

namespace CE
{

    FDockspaceWindow::FDockspaceWindow()
    {
        m_DockspaceClass = FDockspace::StaticClass();
    }

    void FDockspaceWindow::Construct()
    {
        Super::Construct();

        const bool isMac = PlatformMisc::GetCurrentPlatform() == PlatformName::Mac;

        Child(
            FAssignNew(FStyledWidget, borderWidget)
            .Background(FBrush(Color::RGBA(36, 36, 36)))
            .BorderWidth(1.0f)
            .BorderColor(Color::RGBA(15, 15, 15))
            .Padding(Vec4(1, 1, 1, 1) * 2.0f)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .Name("RootStyle")
            (
                FAssignNewDynamic(FDockspace, dockspace, m_DockspaceClass)
                .DestroyWhenEmpty(false)
                .OnWindowSetup([](Ref<FWindow> newWindow, Ref<FDockTabItem> tabItem)
                    {
                        if (Ref<FToolWindow> toolWindow = CastTo<FToolWindow>(newWindow))
                        {
                            toolWindow->ContentPadding(Vec4());
                            toolWindow->Title(tabItem->Title());
                        }
                    })
                .TabWellOverlayWidget(
                    FAssignNew(FHorizontalStack, controlContainer)
                    .HAlign(isMac ? HAlign::Left : HAlign::Right)
                    .VAlign(VAlign::Fill)
                    (
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
                                //OnClickClose();
                                GetContext()->QueueDestroy();
                            })
                        .Name("WindowCloseButton")
                        .Style("Button.WindowClose")
                    )
                )
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                
            ) // End of Child()
        );

        Array<WeakRef<FWindowControlButton>> controlGroup = {
            closeButton,
            minimizeButton,
            maximizeButton
        };

        closeButton->SetControlGroup(controlGroup);
        minimizeButton->SetControlGroup(controlGroup);
        maximizeButton->SetControlGroup(controlGroup);

        if (isMac)
        {
            if (Ref<FDockTabWell> tabWell = dockspace->GetRootTabWell())
            {
                tabWell->Margin(Vec4(60, 0, 0, 0));
            }

            controlContainer->HAlign(HAlign::Left);

            controlContainer->MoveChildToIndex(closeButton, 0);
            controlContainer->MoveChildToIndex(minimizeButton, 1);
            controlContainer->MoveChildToIndex(maximizeButton, 2);

            closeButton->Margin(Vec4(5, 0, -1, 0));
            minimizeButton->Margin(Vec4(0, 0, -1, 0));
        }

        dockspace->SetTabWellWindowHitTest(true);
    }
    
}

