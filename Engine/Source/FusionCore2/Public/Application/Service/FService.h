#pragma once

namespace CE
{
	class FApplication;

    ENUM()
    enum class FServiceTickPhase
    {
	    PumpPlatformEvents,
        DispatchInput,
        PreUpdateSurfaces,
        PostUpdateSurfaces,
		RenderPrepare,
        Render,
		PostRender
    };
    ENUM_CLASS(FServiceTickPhase);

    CLASS(Abstract)
    class FUSIONCORE_API FService : public CE::Object
    {
        CE_CLASS(FService, CE::Object)
    protected:

        FService();
        
    public:

        virtual void OnStart() {}
        virtual void OnShutdown() {}

		virtual void TickService(FServiceTickPhase tickPhase) = 0;

    protected:

        FIELD()
		WeakRef<FApplication> application;

		friend class FApplication;
    };
    
} // namespace CE

#include "FService.rtti.h"
