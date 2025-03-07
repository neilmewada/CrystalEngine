#include "CrystalEditor.h"

namespace CE::Editor
{
    static FusionDemoWindow* instance = nullptr;

    FusionDemoWindow::FusionDemoWindow()
    {

    }

    void FusionDemoWindow::Construct()
    {
        Super::Construct();

        (*this)
            .Title("Fusion Demo")
            .MinimizeEnabled(false)
            .MaximizeEnabled(false)
            .ContentVAlign(VAlign::Center)
            .Content( // Content is a Vertical Stack under the hood!
                FNew(FHorizontalStack)
                .ContentVAlign(VAlign::Center)
                .Gap(5.0f)
                .HAlign(HAlign::Fill)
                (
                    FNew(FStyledWidget)
                    .Background(Color::Red())
                    .Height(25)
                    .FillRatio(1.0f),

                    FNew(FStyledWidget)
                    .Background(Color::Green())
                    .Height(15)
                    .FillRatio(1.0f),

                    FNew(FStyledWidget)
                    .Background(Color::Blue())
                    .Height(30)
                    .FillRatio(2.0f),

                    FNew(FLabel)
                    .Text("This is a label")
                )
            )
        ;
    }

    void FusionDemoWindow::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        if (this == instance)
        {
            instance = nullptr;
        }
    }

    FusionDemoWindow* FusionDemoWindow::Show()
    {
        if (instance != nullptr)
        {
            FNativeContext* nativeContext = static_cast<FNativeContext*>(instance->GetContext());
            PlatformWindow* window = nativeContext->GetPlatformWindow();
            window->SetAlwaysOnTop(true);
            window->Show();
            return instance;
        }

        PlatformWindowInfo info{
            .maximised = false,
            .fullscreen = false,
            .resizable = false,
            .hidden = false,
            .windowFlags = PlatformWindowFlags::Utility | PlatformWindowFlags::DestroyOnClose | PlatformWindowFlags::SkipTaskbar
        };

        Ref<FusionDemoWindow> sampleWindow = FusionApplication::Get()->CreateNativeWindow<FusionDemoWindow>(
            "FusionDemo", "Fusion Demo",
            600, 625, info);

        PlatformWindow* platformWindow = static_cast<FNativeContext*>(sampleWindow->GetContext())->GetPlatformWindow();
        platformWindow->SetAlwaysOnTop(true);

        return sampleWindow.Get();
    }
}

