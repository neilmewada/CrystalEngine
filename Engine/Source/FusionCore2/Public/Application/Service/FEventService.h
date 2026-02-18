#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FEventService : public FService
    {
        CE_CLASS(FEventService, FService)
    protected:

        FEventService();
        
    public:

        void TickService(FServiceTickPhase tickPhase) override;

    protected:

    };
    
} // namespace CE

#include "FEventService.rtti.h"
