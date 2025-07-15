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

        void SetAssetPaths(const Array<Name>& paths) 
        { 
            assetPaths = paths; 
		}

        u32 GetThumbnailResolution() const { return thumbnailResolution; }

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
