#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FParallelAnimation : public FAnimation
    {
        CE_CLASS(FParallelAnimation, FAnimation)
    protected:

        FParallelAnimation();

    public:

        void Add(Ref<FAnimation> animation);
        void Add(Ref<FAnimation> animation, f32 delayBefore);

        void Tick(f32 deltaTime) override;

        void Apply(f32 normalizedTime) override {} // Parallel drives children in Tick

        FIELD()
        Array<Ref<FAnimation>> children;
    };

} // namespace CE

#include "FParallelAnimation.rtti.h"
