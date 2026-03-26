#pragma once

namespace CE
{
    // Internal interface
    class IFInterpolator
    {
    public:

        virtual void Apply(f32 t) = 0;

        virtual ~IFInterpolator() = default;
    };

    template<typename T>
    class FInterpolator : public IFInterpolator
    {
    public:

        FInterpolator(T from, T to, FEasingCurve easing, std::function<void(const T&)> setter)
            : from(MoveTemp(from))
            , to(MoveTemp(to))
            , easing(easing)
            , setter(MoveTemp(setter))
        {}

        void Apply(f32 t) override
        {
            setter(FAnimatable<T>::Lerp(from, to, easing.Evaluate(t)));
        }

    private:
        T from{};
        T to{};
        FEasingCurve easing{};
        std::function<void(const T&)> setter{};
    };

    CLASS()
    class FUSIONCORE_API FTweenAnimation : public FAnimation
    {
        CE_CLASS(FTweenAnimation, FAnimation)
    protected:

        FTweenAnimation();

    public:

        template<typename T>
        void SetInterpolator(T from, T to, FEasingCurve easing, std::function<void(const T&)> setter)
        {
            interpolator.Reset(new FInterpolator<T>(
                MoveTemp(from), MoveTemp(to), easing, MoveTemp(setter)));
        }

        void Apply(f32 normalizedTime) override;

    private:

        UniquePtr<IFInterpolator> interpolator;

        template<typename T>
        friend class FTweenBuilder;
    };

} // namespace CE

#include "FTweenAnimation.rtti.h"
