#include "FusionCore.h"

namespace CE
{

    FSequenceAnimation::FSequenceAnimation()
    {

    }

    void FSequenceAnimation::Add(Ref<FAnimation> animation)
    {
        children.Add(animation);
    }

    void FSequenceAnimation::Add(Ref<FAnimation> animation, f32 delayBefore)
    {
        animation->SetDelay(delayBefore);
        children.Add(animation);
    }

    void FSequenceAnimation::Tick(f32 deltaTime)
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

            currentIndex = reversed ? (int)children.GetSize() - 1 : 0;
            children[currentIndex]->Play();
        }

        if (children.IsEmpty())
            return;

        // Tick the active child
        Ref<FAnimation>& current = children[currentIndex];

        if (!current->IsOwnerValid())
        {
            state = FAnimationState::Completed;
            m_OnComplete.Broadcast();
            return;
        }

        current->Tick(deltaTime);

        if (current->GetState() != FAnimationState::Completed)
            return;

        // Advance to the next child
        const int nextIndex = reversed ? currentIndex - 1 : currentIndex + 1;
        const bool passComplete = reversed ? nextIndex < 0 : nextIndex >= (int)children.GetSize();

        if (!passComplete)
        {
            currentIndex = nextIndex;
            children[currentIndex]->Play();
            return;
        }

        // All children in this pass are done
        switch (loopMode)
        {
        case FAnimationLoopMode::Once:
            state = FAnimationState::Completed;
            m_OnComplete.Broadcast();
            break;

        case FAnimationLoopMode::Loop:
            currentIndex = reversed ? (int)children.GetSize() - 1 : 0;
            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Stop();
            children[currentIndex]->Play();
            break;

        case FAnimationLoopMode::PingPong:
            reversed = !reversed;
            currentIndex = reversed ? (int)children.GetSize() - 1 : 0;
            for (int i = 0; i < children.GetSize(); i++)
                children[i]->Stop();
            children[currentIndex]->Play();
            break;
        }
    }

} // namespace CE
