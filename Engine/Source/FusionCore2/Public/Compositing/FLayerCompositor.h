#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FLayerCompositor : public Object
    {
        CE_CLASS(FLayerCompositor, Object)
    protected:

        FLayerCompositor();
        
    public:


    private:

        StaticArray<FFrameContext, RHI::Limits::MaxSwapChainImageCount> frames{};
    };
    
} // namespace CE

#include "FLayerCompositor.rtti.h"
