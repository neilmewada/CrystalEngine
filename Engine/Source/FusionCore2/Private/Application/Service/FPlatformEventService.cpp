#include "FusionCore.h"

namespace CE
{

    FPlatformEventService::FPlatformEventService()
    {

    }

    void FPlatformEventService::TickService(FServiceTickPhase tickPhase)
    {
        if (tickPhase == FServiceTickPhase::PumpPlatformEvents)
        {
            PlatformApplication::Get()->Tick();
            InputManager::Get().Tick();
		}
    }
} // namespace CE

