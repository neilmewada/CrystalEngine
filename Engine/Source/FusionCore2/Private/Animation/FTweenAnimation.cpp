#include "FusionCore.h"

namespace CE
{

    FTweenAnimation::FTweenAnimation()
    {

    }

    void FTweenAnimation::Apply(f32 normalizedTime)
    {
        if (interpolator)
        {
            interpolator->Apply(normalizedTime);
        }
    }

} // namespace CE
