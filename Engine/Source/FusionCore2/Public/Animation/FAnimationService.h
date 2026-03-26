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

        // - API -

        void Play(Ref<FAnimation> animation, Ref<Object> owner, Name slot);

        void Terminate(Ref<Object> owner, Name slot, bool complete = false);

        void TerminateAll(Ref<Object> owner, bool complete = false);

        // - Tick -

        void TickService(FServiceTickPhase tickPhase) override;

    protected:

        HashMap<Uuid, HashMap<Name, Ref<FAnimation>>> animationSlotsByWidget;

        // Flat list of (ownerUuid, slot) pairs queued for removal this tick
        Array<Pair<Uuid, Name>> animationsToDestroy;

    };
    
} // namespace CE

#include "FAnimationService.rtti.h"
