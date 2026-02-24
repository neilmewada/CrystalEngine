#pragma once

namespace CE
{
    class FLayer;

    CLASS()
    class FUSIONCORE_API FPainter : public Object
    {
        CE_CLASS(FPainter, Object)
    protected:

        FPainter();
        
    public:

        Ref<FLayer> GetOwningLayer() const { return owningLayer.Lock(); }

    private:

        FIELD()
        WeakRef<FLayer> owningLayer;

        friend class FLayer;
    };
    
} // namespace CE

#include "FPainter.rtti.h"
