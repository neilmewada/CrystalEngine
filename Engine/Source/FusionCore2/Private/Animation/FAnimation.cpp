#include "FusionCore.h"

namespace CE
{

    FAnimation::FAnimation()
    {

    }

    void FAnimation::Play()
    {
        elapsedTime = 0.0f;
        delayElapsed = 0.0f;
        reversed = false;
        startFired = false;
        state = FAnimationState::Playing;
    }

    void FAnimation::Pause()
    {
        if (state == FAnimationState::Playing)
        {
            state = FAnimationState::Paused;
        }
    }

    void FAnimation::Resume()
    {
        if (state == FAnimationState::Paused)
        {
            state = FAnimationState::Playing;
        }
    }

    void FAnimation::Stop()
    {
        elapsedTime = 0.0f;
        delayElapsed = 0.0f;
        reversed = false;
        startFired = false;
        state = FAnimationState::Idle;
    }

    void FAnimation::Restart()
    {
        Stop();
        Play();
    }

    void FAnimation::Tick(f32 deltaTime)
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
            delayElapsed += deltaTime;
            if (delayElapsed < delay)
                return;

            startFired = true;
            m_OnStart.Broadcast();
        }

        // Advance elapsed time — speed magnitude controls rate, sign controls direction
        elapsedTime += deltaTime * Math::Abs(speed);

        // Raw normalized time [0, inf)
        f32 rawT = duration > 0.0f ? elapsedTime / duration : 1.0f;

        if (rawT >= 1.0f)
        {
            switch (loopMode)
            {
            case FAnimationLoopMode::Once:
                rawT = 1.0f;
                state = FAnimationState::Completed;
                break;

            case FAnimationLoopMode::Loop:
                elapsedTime = std::fmod(elapsedTime, duration);
                rawT = elapsedTime / duration;
                break;

            case FAnimationLoopMode::PingPong:
                elapsedTime = std::fmod(elapsedTime, duration);
                rawT = elapsedTime / duration;
                reversed = !reversed;
                break;
            }
        }

        // speed < 0 and reversed XOR together to determine final direction
        const bool effectiveReversed = (speed < 0.0f) != reversed;
        const f32 normalizedTime = effectiveReversed ? 1.0f - rawT : rawT;

        Apply(normalizedTime);
        m_OnUpdate.Broadcast(normalizedTime);

        if (state == FAnimationState::Completed)
        {
            m_OnComplete.Broadcast();
        }
    }
} // namespace CE

