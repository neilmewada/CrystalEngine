#include "FusionCore.h"

namespace CE
{

    FAnimationService::FAnimationService()
    {

    }

    void FAnimationService::TickService(FServiceTickPhase tickPhase)
    {
        if (tickPhase == FServiceTickPhase::PreUpdateSurfaces)
        {
			// TODO: Tick animations here
        }
    }
} // namespace CE

