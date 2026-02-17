#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FPlatformEventService : public FService
    {
        CE_CLASS(FPlatformEventService, FService)
    protected:

        FPlatformEventService();
        
    public:

        void TickService(FServiceTickPhase tickPhase) override;

    protected:

    };
    
} // namespace CE

#include "FPlatformEventService.rtti.h"
