#include "FusionCore.h"

namespace CE
{

    FAnimationService::FAnimationService()
    {

    }

    void FAnimationService::Play(Ref<FAnimation> animation, Ref<Object> owner, Name slot)
    {
        if (!animation.IsValid() || !owner.IsValid())
            return;

        Uuid ownerUuid = owner->GetUuid();

        // Stop and destroy any existing animation on this slot
        auto ownerIt = animationSlotsByWidget.Find(ownerUuid);
        if (ownerIt != animationSlotsByWidget.end())
        {
            auto slotIt = ownerIt->second.Find(slot);
            if (slotIt != ownerIt->second.end())
            {
                slotIt->second->Stop();
                slotIt->second->BeginDestroy();
                ownerIt->second.Remove(slot);
            }
        }

        animation->AssignOwner(owner);
        animation->Play();

        animationSlotsByWidget[ownerUuid][slot] = animation;
    }

    void FAnimationService::Terminate(Ref<Object> owner, Name slot, bool complete)
    {
        if (!owner.IsValid())
            return;

        Uuid ownerUuid = owner->GetUuid();

        auto ownerIt = animationSlotsByWidget.Find(ownerUuid);
        if (ownerIt == animationSlotsByWidget.end())
            return;

        auto slotIt = ownerIt->second.Find(slot);
        if (slotIt == ownerIt->second.end())
            return;

        Ref<FAnimation>& animation = slotIt->second;

        if (complete)
            animation->Apply(1.0f);

        animation->Stop();
        animationsToDestroy.Add({ ownerUuid, slot });
    }

    void FAnimationService::TerminateAll(Ref<Object> owner, bool complete)
    {
        if (!owner.IsValid())
            return;

        Uuid ownerUuid = owner->GetUuid();

        auto ownerIt = animationSlotsByWidget.Find(ownerUuid);
        if (ownerIt == animationSlotsByWidget.end())
            return;

        for (auto& [slot, animation] : ownerIt->second)
        {
            if (complete)
                animation->Apply(1.0f);

            animation->Stop();
            animationsToDestroy.Add({ ownerUuid, slot });
        }
    }

    void FAnimationService::TickService(FServiceTickPhase tickPhase)
    {
        if (tickPhase != FServiceTickPhase::PreUpdateSurfaces)
            return;

        const f32 deltaTime = FApplication::Get()->GetDeltaTime();

        for (auto& [ownerUuid, animationsBySlot] : animationSlotsByWidget)
        {
            for (auto& [slot, animation] : animationsBySlot)
            {
                if (!animation->IsOwnerValid())
                {
                    animationsToDestroy.Add({ ownerUuid, slot });
                    continue;
                }

                animation->Tick(deltaTime);

                if (animation->GetState() == FAnimationState::Completed ||
                    animation->GetState() == FAnimationState::Idle)
                {
                    animationsToDestroy.Add({ ownerUuid, slot });
                }
            }
        }

        // Process removals
        for (const auto& [ownerUuid, slot] : animationsToDestroy)
        {
            auto ownerIt = animationSlotsByWidget.Find(ownerUuid);
            if (ownerIt == animationSlotsByWidget.end())
                continue;

            auto slotIt = ownerIt->second.Find(slot);
            if (slotIt == ownerIt->second.end())
                continue;

            slotIt->second->BeginDestroy();
            ownerIt->second.Remove(slot);

            // Clean up the owner entry if no slots remain
            if (ownerIt->second.IsEmpty())
                animationSlotsByWidget.Remove(ownerUuid);
        }

        animationsToDestroy.Clear();
    }

} // namespace CE
