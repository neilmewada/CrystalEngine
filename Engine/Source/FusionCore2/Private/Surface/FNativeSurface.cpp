#include "FusionCore.h"

namespace CE
{

    FNativeSurface::FNativeSurface()
    {

    }

    void FNativeSurface::OnBeforeDestroy()
    {
	    Super::OnBeforeDestroy();

		Shutdown();
    }

    FNativeSurface* FNativeSurface::Create(PlatformWindow* window, const String& name, FSurface* parentSurface)
    {
		if (!FApplication::Get()->IsNativeSurfaceSupported())
        {
            return nullptr;
        }

        Object* outer = parentSurface;

		if (!outer)
        {
            outer = FApplication::Get();
        }
        if (!outer)
        {
            return nullptr;
        }

        FNativeSurface* nativeContext = CreateObject<FNativeSurface>(outer, FixObjectName(name));
		nativeContext->platformWindow = window;

        if (parentSurface)
        {
			parentSurface->AddChildSurface(nativeContext);
        }

		nativeContext->Initialize();

        return nativeContext;
    }

    void FNativeSurface::Initialize()
    {
		scopeId = String::Format("NativeSurface_{}", platformWindow->GetWindowId());

        drawListTag = RPI::RPISystem::Get().GetDrawListTagRegistry()->AcquireTag(scopeId);

        Vec2i screenSize = PlatformApplication::Get()->GetScreenSizeForWindow(platformWindow);

		drawableSize = platformWindow->GetDrawableWindowSize();

        dpiScale = (f32)platformWindow->GetWindowDpi() / 96.0f;

		PlatformApplication::Get()->AddMessageHandler(this);
    }

    void FNativeSurface::Shutdown()
    {
		PlatformApplication::Get()->RemoveMessageHandler(this);
    }

    void FNativeSurface::OnWindowDisplayChanged(PlatformWindow* window, int displayIndex)
    {
        if (window != platformWindow)
			return;

        dpiScale = (f32)platformWindow->GetWindowDpi() / 96.0f;
    }

    void FNativeSurface::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
    {
        if (window != platformWindow)
            return;

        drawableSize = window->GetDrawableWindowSize();
    }

    void FNativeSurface::OnWindowRestored(PlatformWindow* window)
    {
        if (window != platformWindow)
            return;

        drawableSize = window->GetDrawableWindowSize();
    }

    void FNativeSurface::OnWindowMaximized(PlatformWindow* window)
    {
        if (window != platformWindow)
            return;

        drawableSize = window->GetDrawableWindowSize();
    }

} // namespace CE

