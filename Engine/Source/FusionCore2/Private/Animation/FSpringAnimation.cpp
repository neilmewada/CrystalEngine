#include "FusionCore.h"

namespace CE
{

    FSpringAnimation::FSpringAnimation()
    {

    }

    void FSpringAnimation::Tick(f32 dt)
    {
        if (state != FAnimationState::Playing)
            return;

        ZoneScoped;

        if (!owner.Lock().IsValid())
        {
            Stop();
            return;
        }

        // Handle delay
        if (!startFired)
        {
            delayElapsed += dt;
            if (delayElapsed < delay)
                return;

            startFired = true;
            m_OnStart.Broadcast();
        }

        if (!springState)
            return;

        springState->Tick(dt * Math::Abs(speed));
        m_OnUpdate.Broadcast(0.0f);

        if (springState->HasSettled())
        {
            state = FAnimationState::Completed;
            m_OnComplete.Broadcast();
        }
    }

} // namespace CE
