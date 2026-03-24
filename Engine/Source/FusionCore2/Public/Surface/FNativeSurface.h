#pragma once

namespace CE
{
    CLASS()
	class FUSIONCORE_API FNativeSurface : public FSurface, public ApplicationMessageHandler
    {
        CE_CLASS(FNativeSurface, FSurface)
    protected:

        FNativeSurface();
        
    public:

        PlatformWindow* GetPlatformWindow() const { return platformWindow; }

        bool IsFocused() override { return platformWindow != nullptr && platformWindow->IsFocused(); }

        Vec2 ScreenToSurfacePoint(Vec2 position) override;

        // - Events -

		// - Lifecycle -

		static FNativeSurface* Create(PlatformWindow* window, const String& name, FSurface* parentSurface);

        void Initialize() override;

        void Shutdown() override;

		// - Frame Graph -

		void EmplaceFrameAttachments() override;

        void EnqueueScopes() override;

    protected:

		void UpdateDrawableSize();

        void OnWindowDestroyed(PlatformWindow* window) override;
        void OnWindowDisplayChanged(PlatformWindow* window, int displayIndex) override;
        void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
        void OnWindowRestored(PlatformWindow* window) override;
        void OnWindowMaximized(PlatformWindow* window) override;

        void OnWindowKeyboardFocusChanged(PlatformWindow* window, bool gotFocus) override;
        void OnWindowMouseFocusChanged(PlatformWindow* window, bool gotFocus) override;

		PlatformWindow* platformWindow = nullptr;

        RHI::SwapChain* swapChain = nullptr;
    };
    
} // namespace CE

#include "FNativeSurface.rtti.h"
