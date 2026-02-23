#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FAnimationService : public FService
    {
        CE_CLASS(FAnimationService, FService)
    protected:

        FAnimationService();
        
    public:

        void TickService(FServiceTickPhase tickPhase) override;

    };
    
} // namespace CE

#include "FAnimationService.rtti.h"
