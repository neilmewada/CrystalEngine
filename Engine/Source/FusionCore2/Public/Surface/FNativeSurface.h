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

		PlatformWindow* platformWindow = nullptr;
    };
    
} // namespace CE

#include "FNativeSurface.rtti.h"
