#include "FusionCore.h"

namespace CE
{

    FKeyframeAnimation::FKeyframeAnimation()
    {

    }

    void FKeyframeAnimation::Apply(f32 normalizedTime)
    {
        if (track)
        {
            track->Apply(normalizedTime);
        }
    }

} // namespace CE
