#pragma once

namespace CE
{
    CLASS()
	class FUSIONCORE_API FNativeSurface : public FSurface, public ApplicationMessageHandler
    {
        CE_CLASS(FNativeSurface, FSurface)
    protected:

        FNativeSurface();

        void OnBeforeDestroy() override;
        
    public:

		static FNativeSurface* Create(PlatformWindow* window, const String& name, FSurface* parentSurface);

        void Initialize() override;

        void Shutdown() override;

    protected:

		void UpdateDrawableSize();

        void OnWindowDisplayChanged(PlatformWindow* window, int displayIndex) override;

        void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;

        void OnWindowRestored(PlatformWindow* window) override;

        void OnWindowMaximized(PlatformWindow* window) override;

		PlatformWindow* platformWindow = nullptr;
    };
    
} // namespace CE

#include "FNativeSurface.rtti.h"
