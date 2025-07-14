#pragma once

namespace CE::Editor
{
    CLASS(Abstract)
    class EDITORCORE_API AssetThumbnailGen : public Object
    {
        CE_CLASS(AssetThumbnailGen, Object)
    protected:

        AssetThumbnailGen();
        
    public:

        virtual ~AssetThumbnailGen();

        virtual void PrepareJobs(const Array<Name>& assetPaths);

    protected:

        void OnFinish();

    };
    
} // namespace CE

#include "AssetThumbnailGen.rtti.h"
