#pragma once

namespace CE
{
    DECLARE_SCRIPT_EVENT(FAnimationUpdateEvent, f32 normalizedTime);

    ENUM()
    enum class FAnimationLoopMode
    {
	    Once, Loop, PingPong
    };
    ENUM_CLASS(FAnimationLoopMode);

    ENUM()
    enum class FAnimationState
    {
	    Idle, Playing, Paused, Completed
    };
    ENUM_CLASS(FAnimationState);

    CLASS(Abstract)
        class FUSIONCORE_API FAnimation : public Object
    {
        CE_CLASS(FAnimation, Object)
    protected:

        FAnimation();

    public:

        // - Getters / Setters -

        bool IsOwnerValid() const { return owner.IsValid(); }

        FAnimationState GetState() const { return state; }

        f32 GetDuration() const { return duration; }

        f32 GetDelay() const { return delay; }

        void SetDelay(f32 value) { delay = value; }

        void AssignOwner(Ref<Object> owner)
        {
            this->owner = owner;
        }

        // - Playback -

        void Play();
        
    	void Pause();
        
    	void Resume();
        
    	void Stop();
        
    	void Restart();

        // - Tick -

        virtual void Tick(f32 deltaTime);

        virtual void Apply(f32 normalizedTime) = 0;

        // - Events -

        FUSION_EVENT(FVoidEvent, OnStart);
        
    	FUSION_EVENT(FVoidEvent, OnComplete);
        
    	FUSION_EVENT(FAnimationUpdateEvent, OnUpdate);

    protected:

        FIELD()
        f32 duration = 0;

        FIELD()
        f32 delay = 0;

        FIELD()
        f32 speed = 1.0f;

        FIELD()
        FAnimationLoopMode loopMode = FAnimationLoopMode::Once;

        WeakRef<Object> owner;

        f32 elapsedTime = 0.0f;
        f32 delayElapsed = 0.0f;
        bool reversed = false;
        bool startFired = false;
        FAnimationState state = FAnimationState::Idle;
    };
    
} // namespace CE

#include "FAnimation.rtti.h"
