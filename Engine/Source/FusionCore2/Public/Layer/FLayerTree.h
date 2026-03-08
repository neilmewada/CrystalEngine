#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FLayerTree : public Object
    {
        CE_CLASS(FLayerTree, Object)
    public:

        bool IsSyncNeeded() const { return needsSync; }

        void MarkSyncDirty();

        void DoSyncIfNeeded(FWidget* rootWidget);

    protected:

        Ref<FLayer> rootLayer;

        bool needsSync = true;
    };
    
} // namespace CE

#include "FLayerTree.rtti.h"