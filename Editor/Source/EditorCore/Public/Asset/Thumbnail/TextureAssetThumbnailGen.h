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

    };
    
} // namespace CE

#include "TextureAssetThumbnailGen.rtti.h"
