#pragma once

namespace CE::Editor
{
	class AssetImportJob;

	struct AssetImportJobResult
	{
		bool success = false;
		IO::Path sourcePath{};
		IO::Path productPath{};
		String errorMessage{};
	};

	struct AssetUuidNode
	{
		Uuid uuid;
		ClassType* objectType = nullptr;
		Name objectName;
		Array<AssetUuidNode> children;

		void Clear()
		{
			uuid = Uuid();
			objectType = nullptr;
			objectName = nullptr;
			children.Clear();
		}

		bool IsValid() const
		{
			return uuid.IsValid() && objectName.IsValid() && objectType != nullptr;
		}
	};

    CLASS(Abstract, Config = Editor)
    class EDITORCORE_API AssetImporter : public Object
    {
        CE_CLASS(AssetImporter, Object)
    public:
        
        AssetImporter();
        virtual ~AssetImporter();

		void ImportSourceAssetsAsync(const Array<IO::Path>& sourceAssets);

		bool ImportSourceAssetsAsync(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets);

		bool ImportSourceAssets(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets);

		virtual Array<AssetImportJob*> PrepareImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets);

		inline void ClearImportResults() { importResults.Clear(); }
        
		inline bool IsImportInProgress() const { return numJobsInProgress > 0; }

		inline int GetNumJobsLeft() const { return numJobsInProgress; }

		inline const Array<AssetImportJobResult>& GetResults() const { return importResults; }

		inline void SetIncludePaths(const Array<IO::Path>& paths) { this->includePaths = paths; }

		inline void SetLogging(bool enabled) { enableLogging = enabled; }

		inline void SetTargetPlatform(PlatformName targetPlatform) { this->targetPlatform = targetPlatform; }

		inline void SetTempDirectoryPath(const IO::Path& tempDir) { this->tempDirectory = tempDir; }

		virtual Array<Name> GetProductAssetDependencies() { return {}; }

		u32 GetImporterVersion() const { return importerVersion; }

    protected:

		void OnAssetImportJobFinish(AssetImportJob* job);
        
		virtual Array<AssetImportJob*> CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets) = 0;

		FIELD(Config)
		u32 importerVersion = 0;

		SharedMutex mutex{};
		int numJobsInProgress = 0;
		bool enableLogging = false;
		IO::Path tempDirectory{};

		PlatformName targetPlatform;

		Array<AssetImportJobResult> importResults{};

		Array<IO::Path> includePaths{};

		friend class AssetImportJob;
    };

	class EDITORCORE_API AssetImportJob : public Job
	{
	public:
		typedef AssetImportJob Self;
		typedef Job Super;

		AssetImportJob(AssetImporter* importer, const IO::Path& sourcePath, const IO::Path& productPath)
			: Job(true)
			, importer(importer)
			, sourcePath(sourcePath)
			, productPath(productPath)
		{
			targetPlatform = PlatformMisc::GetCurrentPlatform();
		}

		void Finish() override;

		void Process() override;

		virtual bool ProcessAsset(const Ref<Bundle>& bundle) = 0;

		virtual Array<Name> PrepareProductAssetDependencies();

		inline bool Succeeded() const { return success; }
		inline const String& GetErrorMessage() const { return errorMessage; }

		inline bool IsGeneratingDistributionAsset() const 
		{
			return generateDistributionAsset;
		}

		inline const IO::Path& GetSourcePath() const { return sourcePath; }
		inline const IO::Path& GetProductPath() const { return productPath; }

	protected:

		struct UuidFetcher
		{
			ClassType* clazz = nullptr;
			Uuid* outUuid = nullptr;
			Array<UuidFetcher> children;
		};

		void FetchObjectUuids(const Ref<Bundle>& bundle, const Array<UuidFetcher>& uuidFetchers);

		void BuildUuidTree(Ref<Bundle> bundle);

		Array<IO::Path> includePaths{};

		PlatformName targetPlatform;

		String sourceAssetRelativePath = "";
		String bundlePath = "";

		b8 generateDistributionAsset = false;
		b8 isGameAsset = false;
		b8 regenerateUuid = false;
		
		IO::Path sourcePath{};
		IO::Path productPath{};
		IO::Path editorProductPath{};
		
		String errorMessage = "";

		AssetImporter* importer = nullptr;

	private:

		bool success = false;
		AssetUuidNode rootNode{};

		friend class AssetImporter;
	};

} // namespace CE::Editor

#include "AssetImporter.rtti.h"
