#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ComputeShaderAssetImporter : public AssetImporter
    {
        CE_CLASS(ComputeShaderAssetImporter, AssetImporter)
    protected:

        ComputeShaderAssetImporter();
        
    public:

        virtual ~ComputeShaderAssetImporter();

        Array<AssetImportJob*> CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets) override;

    };

    class EDITORCORE_API ComputeShaderAssetImportJob : public AssetImportJob
    {
    public:
        typedef ComputeShaderAssetImportJob Self;
        typedef AssetImportJob Super;

        ComputeShaderAssetImportJob(AssetImporter* importer, const IO::Path& sourcePath, const IO::Path& outPath)
            : Super(importer, sourcePath, outPath)
        {

        }

        virtual bool ProcessAsset(const Ref<Bundle>& bundle) override;

    };
    
} // namespace CE

#include "ComputeShaderAssetImporter.rtti.h"
