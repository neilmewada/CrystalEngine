#include "FusionCore.h"

namespace CE
{

    FDockspaceWindow::FDockspaceWindow()
    {
        dockspaceClass = FDockspace::StaticClass();
    }

    void FDockspaceWindow::Construct()
    {
        Super::Construct();

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
                FAssignNewDynamic(FDockspace, dockspace, dockspaceClass)
                .DockspaceType(FDockTypeMask::Major)
                .AllowDocking(true)
                .DestroyWhenEmpty(false)
                .AllowSplitting(true)
                .DetachedWindowClass(Self::StaticClass())
                .OnWindowSetup([](Ref<FWindow> newWindow, Ref<FDockTabItem> tabItem)
                    {
                        if (newWindow->IsOfType<FToolWindow>())
                        {
                            Ref<FToolWindow> toolWindow = CastTo<FToolWindow>(newWindow);
                            toolWindow->ContentPadding(Vec4());
                            toolWindow->Title(tabItem->Title());
                        }
                    })
                .TabWellBackgroundWidget(
                    FAssignNew(FTitleBar, titleBar)
                    .Background(FBrush(Color::RGBA(26, 26, 26)))
                    .Height(35)
                    .HAlign(HAlign::Fill)
                )
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                
            ) // End of Child()
        );

        dockspace->GetTabWellParent()->AddChild(
            FNew(FHorizontalStack)
            .HAlign(HAlign::Fill)
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
        );
    }
    
}

