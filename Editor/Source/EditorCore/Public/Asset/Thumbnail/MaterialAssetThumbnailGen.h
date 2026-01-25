#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API MaterialAssetThumbnailGen : public AssetThumbnailGen
    {
        CE_CLASS(MaterialAssetThumbnailGen, AssetThumbnailGen)
    protected:

        MaterialAssetThumbnailGen();
        
    public:

        virtual ~MaterialAssetThumbnailGen();

        bool StartProcessing() override;

        bool IsProcessing() override;

        bool IsValidForAssetType(SubClass<Asset> assetClass) override;

    private:

        void OnJobFinished(Ref<SceneRenderer> sceneRenderer);

        int totalJobs = 0;
        Array<Ref<SceneRenderer>> sceneRenderers;
    };
    
} // namespace CE

#include "MaterialAssetThumbnailGen.rtti.h"
