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

    };
    
} // namespace CE

#include "MaterialAssetThumbnailGen.rtti.h"
