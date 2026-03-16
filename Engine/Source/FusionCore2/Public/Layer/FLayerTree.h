#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FLayerTree : public Object
    {
        CE_CLASS(FLayerTree, Object)
    public:

        bool IsSyncNeeded() const { return needsSync; }

        void MarkSyncNeeded();

        void DoSyncIfNeeded(FWidget* rootWidget);

        void DoPaintIfNeeded();

        Ref<FLayer> FindLayerForWidget(Uuid widgetUuid);

        Ref<FLayer> GetRootLayer() { return rootLayer; }

        void UpdateLayerSrg(u32 frameIndex);

    protected:

        void SyncWidget(FWidget* widget, FLayer* parentLayer, HashSet<Uuid>& visited);

        Ref<FLayer> rootLayer;

        FAffineTransform rootLayerTransform;

        HashMap<Uuid, Ref<FLayer>> widgetUuidToLayerMap;

        bool needsSync = true;
    };
    
} // namespace CE

#include "FLayerTree.rtti.h"