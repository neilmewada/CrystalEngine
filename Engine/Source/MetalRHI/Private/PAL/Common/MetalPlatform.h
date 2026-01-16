#pragma once

#include "QuartzCore/CAMetalLayer.h"

namespace CE::Metal
{
    class Texture;
    class Sampler;

    class MetalPlatformBase
    {
        CE_STATIC_CLASS(MetalPlatformBase)
    public:
        
        
    };
}

#if PLATFORM_MAC
#include "PAL/Mac/MetalMacPlatform.h"
#endif

#include "PAL/SDL/MetalSDLPlatform.h"


