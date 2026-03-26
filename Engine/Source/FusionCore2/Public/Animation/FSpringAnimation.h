#pragma once

namespace CE
{
    // Internal interface — not a CE::Object, never exposed directly
    class IFSpringState
    {
    public:
        virtual void Tick(f32 dt) = 0;
        virtual bool HasSettled() const = 0;
        virtual ~IFSpringState() = default;
    };

    template<typename T>
    class FSpringState : public IFSpringState
    {
    public:

        FSpringState(T current, T target, f32 stiffness, f32 damping, f32 settleEpsilon,
                     std::function<void(const T&)> setter)
            : current(MoveTemp(current))
            , velocity(FAnimatable<T>::Identity())
            , target(MoveTemp(target))
            , stiffness(stiffness)
            , damping(damping)
            , settleEpsilon(settleEpsilon)
            , setter(MoveTemp(setter))
        {}

        void SetTarget(T newTarget) { target = MoveTemp(newTarget); }

        void Tick(f32 dt) override
        {
            using A = FAnimatable<T>;

            // Semi-implicit Euler:
            // force    = stiffness * (target - current) - damping * velocity
            // velocity += force * dt
            // current  += velocity * dt
            const T error    = A::Add(target,   A::Scale(current,  -1.0f));
            const T force    = A::Add(A::Scale(error, stiffness), A::Scale(velocity, -damping));
            velocity         = A::Add(velocity, A::Scale(force,    dt));
            current          = A::Add(current,  A::Scale(velocity, dt));
            setter(current);
        }

        bool HasSettled() const override
        {
            using A = FAnimatable<T>;
            const T error = A::Add(target, A::Scale(current, -1.0f));
            const f32 eps2 = settleEpsilon * settleEpsilon;
            return A::SquaredMagnitude(error)    < eps2 &&
                   A::SquaredMagnitude(velocity) < eps2;
        }

    private:
        T current;
        T velocity;
        T target;
        f32 stiffness;
        f32 damping;
        f32 settleEpsilon;
        std::function<void(const T&)> setter;
    };

    CLASS()
    class FUSIONCORE_API FSpringAnimation : public FAnimation
    {
        CE_CLASS(FSpringAnimation, FAnimation)
    protected:

        FSpringAnimation();

    public:

        template<typename T>
        void SetSpring(T current, T target, std::function<void(const T&)> setter)
        {
            springState.Reset(new FSpringState<T>(
                MoveTemp(current), MoveTemp(target),
                stiffness, damping, settleEpsilon,
                MoveTemp(setter)));
        }

        template<typename T>
        void SetTarget(T newTarget)
        {
            if (auto* s = dynamic_cast<FSpringState<T>*>(springState.Get()))
                s->SetTarget(MoveTemp(newTarget));
        }

        void Tick(f32 dt) override;
        void Apply(f32) override {} // Unused — spring drives its own apply in Tick

    protected:

        FIELD()
        f32 stiffness = 200.0f;

        FIELD()
        f32 damping = 20.0f;

        FIELD()
        f32 settleEpsilon = 0.001f;

    private:

        UniquePtr<IFSpringState> springState; // Runtime-only — holds current, velocity, target, setter

        template<typename T>
        friend class FSpringBuilder;
    };

} // namespace CE

#include "FSpringAnimation.rtti.h"
