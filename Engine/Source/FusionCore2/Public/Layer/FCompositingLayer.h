#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FCompositingLayer : public FLayer
    {
        CE_CLASS(FCompositingLayer, FLayer)
    protected:

        FCompositingLayer();
        
    public:


    protected:

        friend class FLayerTree;
    };
    
} // namespace CE

#include "FCompositingLayer.rtti.h"
