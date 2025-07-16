#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API TextureAssetThumbnailGen : public AssetThumbnailGen
    {
        CE_CLASS(TextureAssetThumbnailGen, AssetThumbnailGen)
    protected:

        TextureAssetThumbnailGen();
        
    public:

        virtual ~TextureAssetThumbnailGen();

        bool IsValidForAssetType(SubClass<Asset> assetClass) override;

        bool IsProcessing() override;

        bool StartProcessing() override;

    private:

        Array<Ref<CE::Texture>> textureAssets;
        Ref<CE::Shader> blitShaderAsset;

        bool started = false;
        JobCompletion jobCompletion = JobCompletion();

    };
    
} // namespace CE

#include "TextureAssetThumbnailGen.rtti.h"
