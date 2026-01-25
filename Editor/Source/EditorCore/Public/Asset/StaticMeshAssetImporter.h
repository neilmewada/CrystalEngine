#pragma once

namespace CE::Editor
{
    CLASS(Config = Editor)
    class EDITORCORE_API StaticMeshAssetImporter : public AssetImporter
    {
        CE_CLASS(StaticMeshAssetImporter, AssetImporter)
    public:
        
        virtual Array<AssetImportJob*> CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets) override;

    	Array<Name> GetProductAssetDependencies() override;

    private:

		FIELD(Config)
		bool optimizeMeshes = true;
		
		FIELD(Config)
		bool generateUV1 = false;

    };

	class EDITORCORE_API StaticMeshAssetImportJob : public AssetImportJob
	{
	public:
		typedef StaticMeshAssetImportJob Self;
		typedef AssetImportJob Super;

		StaticMeshAssetImportJob(AssetImporter* importer, const IO::Path& sourcePath, const IO::Path& outPath)
			: Super(importer, sourcePath, outPath)
		{

		}

		Array<Name> PrepareProductAssetDependencies() override;

		virtual bool ProcessAsset(const Ref<Bundle>& bundle) override;

	private:

		bool optimizeMeshes = true;
		bool generateUV1 = false;

		friend class StaticMeshAssetImporter;
	};
    
} // namespace CE::Editor

#include "StaticMeshAssetImporter.rtti.h"
