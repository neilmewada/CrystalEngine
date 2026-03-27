#include "FusionCore.h"

namespace CE
{

    FParallelAnimation::FParallelAnimation()
    {

    }

    void FParallelAnimation::Add(Ref<FAnimation> animation)
    {
        children.Add(animation);
    }

    void FParallelAnimation::Add(Ref<FAnimation> animation, f32 delayBefore)
    {
        animation->SetDelay(delayBefore);
        children.Add(animation);
    }

    void FParallelAnimation::Tick(f32 deltaTime)
    {
    	if (state != FAnimationState::Playing)
            return;

        ZoneScoped;

        if (!owner.Lock().IsValid())
        {
            Stop();
            return;
        }

        // Handle own delay
        if (!startFired)
        {
            delayElapsed += deltaTime;
            if (delayElapsed < delay)
                return;

            startFired = true;
            m_OnStart.Broadcast();

            if (children.IsEmpty())
            {
                state = FAnimationState::Completed;
                m_OnComplete.Broadcast();
                return;
            }

            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Play();
        }

        if (children.IsEmpty())
            return;

        // Tick all active children
        bool anyPlaying = false;
        for (int i = 0; i < children.GetSize(); i++)
        {
            Ref<FAnimation>& child = children[i];

            if (!child->IsOwnerValid())
            {
                state = FAnimationState::Completed;
                m_OnComplete.Broadcast();
                return;
            }

            if (child->GetState() == FAnimationState::Playing)
            {
                child->Tick(deltaTime);
            }
            if (child->GetState() != FAnimationState::Completed)
            {
                anyPlaying = true;
            }
        }

        if (anyPlaying)
            return;

        // All children are done
        switch (loopMode)
        {
        case FAnimationLoopMode::Once:
            state = FAnimationState::Completed;
            m_OnComplete.Broadcast();
            break;

        case FAnimationLoopMode::Loop:
            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Stop();
            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Play();
            break;

        case FAnimationLoopMode::PingPong:
            reversed = !reversed;
            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Stop();
            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Play();
            break;
        }
    }

} // namespace CE
