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

        bool IsValidForAssetType(SubClass<Asset> assetClass) override;

    private:

        Array<Ref<CE::MaterialInterface>> materialAssets;
    };
    
} // namespace CE

#include "MaterialAssetThumbnailGen.rtti.h"
