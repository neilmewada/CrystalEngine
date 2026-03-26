#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FSequenceAnimation : public FAnimation
    {
        CE_CLASS(FSequenceAnimation, FAnimation)
    protected:

        FSequenceAnimation();

    public:

        void Add(Ref<FAnimation> animation);
        void Add(Ref<FAnimation> animation, f32 delayBefore);

        void Tick(f32 deltaTime) override;

        void Apply(f32 normalizedTime) override {} // Sequence drives children in Tick

        FIELD()
        Array<Ref<FAnimation>> children;

    private:

        int currentIndex = 0;
    };

} // namespace CE

#include "FSequenceAnimation.rtti.h"
