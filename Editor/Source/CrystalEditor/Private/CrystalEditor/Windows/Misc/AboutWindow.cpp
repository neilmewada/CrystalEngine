#include "CrystalEditor.h"

namespace CE::Editor
{
    static AboutWindow* instance = nullptr;

    AboutWindow::AboutWindow()
    {

    }

    void AboutWindow::Construct()
    {
        Super::Construct();

        titleBar->Height(30);
        titleBarLabel->FontSize(13);

        constexpr auto splashImage = "/Editor/Assets/UI/Splash";

        instance = this;

        FBrush splash = FBrush(splashImage);
        splash.SetBrushTiling(FBrushTiling::None);
        splash.SetImageFit(FImageFit::Cover);
        splash.SetBrushPosition(Vec2(0.5f, 0.5f));

        // Factors:
        // VAlign, HAlign, TileX, TileY, Width, Height

        (*this)
	        .Title("About Crystal Editor")
	        .MinimizeEnabled(false)
			.MaximizeEnabled(false)
			.ContentPadding(Vec4(1, 1, 1, 1) * 20)
	        .Content(
                FNew(FImage)
                .Background(splash)
                .HAlign(HAlign::Fill)
                .Height(350)
                .Margin(Vec4(0, 0, 0, 10)),

                FNew(FLabel)
                .FontSize(13)
                .Text("CrystalEditor")
                .HAlign(HAlign::Left),

                FNew(FLabel)
                .FontSize(10)
                .Text("Version: " CE_ENGINE_VERSION_STRING_SHORT " pre-alpha [Build " CE_TOSTRING(CE_VERSION_BUILD) "]")
                .HAlign(HAlign::Left),

                FNew(FLabel)
                .FontSize(10)
                .Text("Platform: " + PlatformMisc::GetOSVersionString())
                .HAlign(HAlign::Left),

                FNew(FLabel)
                .FontSize(10)
                .Text("Icons by flaticon.com")
                .HAlign(HAlign::Left)
	        )
        ;
    }

    void AboutWindow::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (this == instance)
        {
            instance = nullptr;
        }
    }

    AboutWindow* AboutWindow::Show()
    {
        if (instance != nullptr)
        {
            Ref<FNativeContext> nativeContext = CastTo<FNativeContext>(instance->GetContext());
            PlatformWindow* window = nativeContext->GetPlatformWindow();
            window->SetAlwaysOnTop(true);
        	window->Show();
            return instance;
        }

        PlatformWindowInfo info{
			.maximised = false,
			.fullscreen = false,
			.resizable = false,
			.hidden = true,
			.windowFlags = PlatformWindowFlags::Utility | PlatformWindowFlags::DestroyOnClose | PlatformWindowFlags::SkipTaskbar
        };

        f32 scaling = PlatformApplication::Get()->GetSystemDpi() / 96.0f;
    	scaling *= FusionApplication::Get()->GetDefaultScalingFactor();
#if PLATFORM_MAC
    	scaling = 1;
#endif

        PlatformWindow* window = PlatformApplication::Get()->CreatePlatformWindow("About", (u32)(600 * scaling), (u32)(500 * scaling), info);
        window->SetAlwaysOnTop(true);
        window->SetBorderless(true);

        FRootContext* rootContext = FusionApplication::Get()->GetRootContext();
        Ref<FFusionContext> parentContext = rootContext;
        if (rootContext->GetChildContexts().NotEmpty())
        {
            // FRootContext should have only 1 NativeContext which is the primary Native Window
            parentContext = rootContext->GetChildContexts().GetFirst();
        }

        FNativeContext* context = FNativeContext::Create(window, "AboutWindow", parentContext.Get());
        parentContext->AddChildContext(context);

        AboutWindow* aboutWindow = nullptr;

        FAssignNewOwned(AboutWindow, aboutWindow, context);
        context->SetOwningWidget(aboutWindow);

        window->Show();

        return aboutWindow;
    }
}

