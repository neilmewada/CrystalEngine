#pragma once

namespace CE::Editor
{
    CLASS(Abstract, Config = Editor)
    class EDITORCORE_API AssetThumbnailGen : public Object
    {
        CE_CLASS(AssetThumbnailGen, Object)
    protected:

        AssetThumbnailGen();
        
    public:

        virtual ~AssetThumbnailGen();

        virtual bool StartProcessing() = 0;

        virtual bool IsValidForAssetType(SubClass<Asset> assetClass) = 0;

		//! @brief Returns true if the thumbnail generation is currently processing.
        virtual bool IsProcessing() { return false; }

        void SetAssetPaths(const Array<Name>& paths) 
        { 
            assetPaths = paths; 
		}

        const Array<Name>& GetAssetPaths() const
        {
            return assetPaths;
        }

        u32 GetThumbnailResolution() const { return thumbnailResolution; }

        static Name GetThumbnailPath(Name assetPath);

        static bool SaveThumbnailToDisk(const CMImage& rawImage, const Name& assetPath);

        static void SaveThumbnailToDisk(void* rgbaImageData, u32 width, u32 height, const Name& assetPath, bool compress = true);

    protected:

        void OnFinish();

        Array<Name> assetPaths;

        FIELD(Config)
        u32 thumbnailResolution = 128;

    public:

        ScriptEvent<void(Ref<AssetThumbnailGen>)> onFinishEvent;

    };
    
} // namespace CE

#include "AssetThumbnailGen.rtti.h"
