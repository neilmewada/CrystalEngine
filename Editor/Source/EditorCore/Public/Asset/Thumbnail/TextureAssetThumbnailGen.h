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

        bool StartProcessing() override;

    private:

        Array<Ref<CE::Texture>> textureAssets;
        Ref<CE::Shader> blitShaderAsset;

        JobCompletion jobCompletion = JobCompletion();

    };
    
} // namespace CE

#include "TextureAssetThumbnailGen.rtti.h"
