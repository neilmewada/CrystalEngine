#include "FusionCore.h"

namespace CE
{

    FEventService::FEventService()
    {

    }

    void FEventService::TickService(FServiceTickPhase tickPhase)
    {
        if (tickPhase == FServiceTickPhase::PumpPlatformEvents)
        {
            PlatformApplication::Get()->Tick();
            InputManager::Get().Tick();
		}
        else if (tickPhase == FServiceTickPhase::DispatchInput)
        {
	        
        }
    }
} // namespace CE

