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

        FNativeSurface* nativeSurface = CreateObject<FNativeSurface>(outer, FixObjectName(name));
		nativeSurface->platformWindow = window;

        if (parentSurface)
        {
			parentSurface->AddChildSurface(nativeSurface);
        }

		nativeSurface->Initialize();

        return nativeSurface;
    }

    void FNativeSurface::Initialize()
    {
		scopeId = String::Format("NativeSurface_{}", platformWindow->GetWindowId());

        drawListTag = RPI::RPISystem::Get().GetDrawListTagRegistry()->AcquireTag(scopeId);

        UpdateDrawableSize();

		PlatformApplication::Get()->AddMessageHandler(this);

        FApplication::Get()->AddSurface(this);
    }

    void FNativeSurface::Shutdown()
    {
		FApplication::Get()->RemoveSurface(this);

		RPISystem::Get().GetDrawListTagRegistry()->ReleaseTag(drawListTag);

		PlatformApplication::Get()->RemoveMessageHandler(this);
    }

    void FNativeSurface::EmplaceFrameAttachments()
    {
		auto scheduler = RHI::FrameScheduler::Get();

        RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

        for (Ref<FSurface> childSurface : childrenSurfaces)
        {
			childSurface->EmplaceFrameAttachments();
        }
    }

    void FNativeSurface::UpdateDrawableSize()
    {
        dpiScale = platformWindow->GetDpiScaling();
		if (dpiScale <= 0)
            return;

        drawableSize = platformWindow->GetDrawableWindowSize();

        Vec2 newAvailableSize = drawableSize.ToVec2() / dpiScale;

        if (newAvailableSize != availableSize && rootWidget)
        {
            

            rootWidget->MarkLayoutDirty();
            rootWidget->MarkPaintDirty();
        }

		availableSize = newAvailableSize;
    }

    void FNativeSurface::OnWindowDisplayChanged(PlatformWindow* window, int displayIndex)
    {
        if (window != platformWindow)
			return;

        UpdateDrawableSize();
    }

    void FNativeSurface::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
    {
        if (window != platformWindow)
            return;

        UpdateDrawableSize();
    }

    void FNativeSurface::OnWindowRestored(PlatformWindow* window)
    {
        if (window != platformWindow)
            return;

        UpdateDrawableSize();
    }

    void FNativeSurface::OnWindowMaximized(PlatformWindow* window)
    {
        if (window != platformWindow)
            return;

        UpdateDrawableSize();
    }

} // namespace CE

