#pragma once

namespace CE
{
    // Internal interface
    class IFKeyframeTrack
    {
    public:

        virtual void Apply(f32 t) = 0;

        virtual ~IFKeyframeTrack() = default;
    };

    template<typename T>
    class FKeyframeTrack : public IFKeyframeTrack
    {
    public:

        struct FKeyframe
        {
            f32          time  = 0.0f;
            T            value = {};
            FEasingCurve easing{};
        };

        FKeyframeTrack(Delegate<void(const T&)> setter)
            : setter(MoveTemp(setter))
        {}

        void AddKey(f32 time, T value, FEasingCurve easing = {})
        {
            keyframes.Add({ time, MoveTemp(value), easing });
            dirty = true;
        }

        void Apply(f32 t) override
        {
            if (keyframes.IsEmpty() || !setter.IsValid())
                return;

            // Sort by time if keys were added out of order
            if (dirty)
            {
                keyframes.Sort([](const FKeyframe& a, const FKeyframe& b)
                {
                    return a.time < b.time;
                });
                dirty = false;
            }

            // Before or at the first keyframe
            if (t <= keyframes[0].time)
            {
                setter(keyframes[0].value);
                return;
            }

            // After or at the last keyframe
            if (t >= keyframes[keyframes.GetSize() - 1].time)
            {
                setter(keyframes[keyframes.GetSize() - 1].value);
                return;
            }

            // Find the right boundary — first keyframe with time > t
            int right = 1;
            while (right < keyframes.GetSize() && keyframes[right].time <= t)
                right++;

            const int left = right - 1;

            // Compute segment-local normalized t
            const f32 segStart = keyframes[left].time;
            const f32 segEnd   = keyframes[right].time;
            const f32 segT     = (t - segStart) / (segEnd - segStart);

            // Easing is defined on the left keyframe (controls the segment going forward)
            const f32 easedT = keyframes[left].easing.Evaluate(segT);

            setter(FAnimatable<T>::Lerp(keyframes[left].value, keyframes[right].value, easedT));
        }

    private:

        Array<FKeyframe>        keyframes{};
        Delegate<void(const T&)> setter{};
        bool                    dirty = false;
    };

    CLASS()
    class FUSIONCORE_API FKeyframeAnimation : public FAnimation
    {
        CE_CLASS(FKeyframeAnimation, FAnimation)
    protected:

        FKeyframeAnimation();

    public:

        // Creates the typed track, stores it, and returns a pointer so the
        // caller can add keys via track->AddKey(...)
        template<typename T>
        FKeyframeTrack<T>* SetTrack(Delegate<void(const T&)> setter)
        {
            FKeyframeTrack<T>* track = new FKeyframeTrack<T>(MoveTemp(setter));
            this->track.Reset(track);
            return track;
        }

        void Apply(f32 normalizedTime) override;

    private:

        UniquePtr<IFKeyframeTrack> track;
    };

} // namespace CE

#include "FKeyframeAnimation.rtti.h"
