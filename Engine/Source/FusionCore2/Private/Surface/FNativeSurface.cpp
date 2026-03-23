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

    Vec2 FNativeSurface::ScreenToSurfacePoint(Vec2 position)
    {
        return (position - platformWindow->GetWindowPosition().ToVec2()) / dpiScale;
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
        Super::Initialize();

		scopeId = String::Format("NativeSurface_{}", platformWindow->GetWindowId());
        attachmentId = scopeId;

        drawListTag = RPI::RPISystem::Get().GetDrawListTagRegistry()->AcquireTag(scopeId);

        for (u32 i = 0; i < renderSnapshots.GetSize(); i++)
        {
            renderSnapshots[i] = new FRenderSnapshot(drawListTag);
        }

        UpdateDrawableSize();

		PlatformApplication::Get()->AddMessageHandler(this);

        FApplication::Get()->AddSurface(this);

        RHI::SwapChainDescriptor desc{};
        desc.frameBufferOnly = true;
        desc.imageCount = 2;
        desc.useMailboxMode = true;
        desc.preferredFormats = { RHI::Format::R8G8B8A8_UNORM, RHI::Format::B8G8R8A8_UNORM };

        swapChain = gDynamicRHI->CreateSwapChain(platformWindow, desc);
    }

    void FNativeSurface::Shutdown()
    {
        Super::Shutdown();

        delete swapChain; swapChain = nullptr;

		FApplication::Get()->RemoveSurface(this);

		RPI::RPISystem::Get().GetDrawListTagRegistry()->ReleaseTag(drawListTag);

		PlatformApplication::Get()->RemoveMessageHandler(this);
    }

    void FNativeSurface::EmplaceFrameAttachments()
    {
        ZoneScoped;

		auto scheduler = RHI::FrameScheduler::Get();

        RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

        if (swapChain)
        {
            attachmentDatabase.EmplaceFrameAttachment(attachmentId, swapChain);
        }

        for (Ref<FSurface> childSurface : childrenSurfaces)
        {
			childSurface->EmplaceFrameAttachments();
        }
    }

    void FNativeSurface::EnqueueScopes()
    {
        ZoneScoped;

        auto scheduler = RHI::FrameScheduler::Get();

        scheduler->BeginScope(scopeId);
        {
            RHI::ImageScopeAttachmentDescriptor swapChainAttachment{};
            swapChainAttachment.attachmentId = attachmentId;
            swapChainAttachment.loadStoreAction.loadAction = AttachmentLoadAction::Clear;
            swapChainAttachment.loadStoreAction.storeAction = AttachmentStoreAction::Store;
            swapChainAttachment.multisampleState.sampleCount = 1;
            scheduler->UseAttachment(swapChainAttachment, RHI::ScopeAttachmentUsage::Color, RHI::ScopeAttachmentAccess::ReadWrite);

            scheduler->PresentSwapChain(swapChain);
        }
        scheduler->EndScope();

        for (Ref<FSurface> childSurface : childrenSurfaces)
        {
            childSurface->EnqueueScopes();
        }
    }

    void FNativeSurface::UpdateDrawableSize()
    {
        ZoneScoped;

        dpiScale = platformWindow->GetDpiScaling();
		if (dpiScale <= 0)
            return;

        drawableSize = platformWindow->GetDrawableWindowSize();

        Vec2 newAvailableSize = drawableSize.ToVec2() / dpiScale;
        bool sizeChanged = false;

        if (newAvailableSize != availableSize && rootWidget)
        {
            sizeChanged = true;

            rootWidget->MarkLayoutDirty();
            rootWidget->MarkPaintDirty();
        }

		availableSize = newAvailableSize;

        if (sizeChanged)
        {
            OnSurfaceResize();
        }
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

    void FNativeSurface::OnWindowKeyboardFocusChanged(PlatformWindow* window, bool gotFocus)
    {
        if (window != platformWindow)
            return;

        if (FEventService* eventService = FApplication::Get()->GetService<FEventService>().Get())
        {
            if (gotFocus)
                eventService->FocusSurface(this);
        }
    }

    void FNativeSurface::OnWindowMouseFocusChanged(PlatformWindow* window, bool gotFocus)
    {
        if (window != platformWindow)
            return;

        if (FEventService* eventService = FApplication::Get()->GetService<FEventService>().Get())
        {
            if (gotFocus)
                eventService->FocusSurface(this);
        }
    }

} // namespace CE

