#pragma once

namespace CE
{
    class FLayerTree;

    CLASS()
    class FUSIONCORE_API FLayer : public Object
    {
        CE_CLASS(FLayer, Object)
    public:

        FLayer();

    protected:

        FIELD()
        WeakRef<FLayerTree> ownerTree;

        friend class FLayerTree;
    };

} // namespace CE

#include "FLayer.rtti.h"