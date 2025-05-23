#include "EditorCore.h"

namespace CE::Editor
{

    AssetImporter::AssetImporter()
    {
		targetPlatform = PlatformMisc::GetCurrentPlatform();
    }

    AssetImporter::~AssetImporter()
    {
        
    }

	void AssetImporter::ImportSourceAssetsAsync(const Array<IO::Path>& sourceAssets)
	{
		Array<IO::Path> productAssets{};

		for (const auto& sourceAsset : sourceAssets)
		{
			productAssets.Add(sourceAsset.ReplaceExtension(".casset"));
		}

		ImportSourceAssetsAsync(sourceAssets, productAssets);
	}

	bool AssetImporter::ImportSourceAssetsAsync(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets)
	{
		if (productAssets.GetSize() != sourceAssets.GetSize())
		{
			CE_LOG(Error, All, "Failed to ImportSourceAssetsAsync(): productAssets array does not match the size of sourceAssets");
			return false;
		}

		auto jobs = CreateImportJobs(sourceAssets, productAssets);

		for (auto job : jobs)
		{
			job->includePaths = includePaths;
			job->targetPlatform = targetPlatform;
			job->SetAutoDelete(true);
			
			job->Start();
		}

		return true;
	}

	bool AssetImporter::ImportSourceAssets(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets)
	{
		if (productAssets.GetSize() != sourceAssets.GetSize())
		{
			CE_LOG(Error, All, "Failed to ImportSourceAssetsAsync(): productAssets array does not match the size of sourceAssets");
			return false;
		}

		auto jobs = CreateImportJobs(sourceAssets, productAssets);

		for (auto job : jobs)
		{
			job->includePaths = includePaths;
			job->targetPlatform = targetPlatform;
			job->SetAutoDelete(false);

			job->Process();
			job->Finish();
			delete job;
		}
		jobs.Clear();

		return true;
	}

	Array<AssetImportJob*> AssetImporter::PrepareImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets)
	{
		if (productAssets.GetSize() != sourceAssets.GetSize())
		{
			CE_LOG(Error, All, "Failed to PrepareImportJobs(): productAssets array does not match the size of sourceAssets");
			return {};
		}

		auto jobs = CreateImportJobs(sourceAssets, productAssets);

		for (auto job : jobs)
		{
			job->regenerateUuid = false;
			job->includePaths = includePaths;
			job->targetPlatform = targetPlatform;
			job->SetAutoDelete(true);
		}

		return jobs;
	}

	void AssetImporter::OnAssetImportJobFinish(AssetImportJob* job)
	{
		LockGuard guard{ mutex };

		AssetImportJobResult importResult{};
		importResult.success = job->success;
		importResult.sourcePath = job->sourcePath;
		importResult.productPath = job->productPath;
		importResult.errorMessage = job->errorMessage;

		if (!job->success)
		{
			CE_LOG(Error, All, "Failed to import asset {}. Error: {}", job->sourcePath, job->errorMessage);
		}
		else if (enableLogging)
		{
			CE_LOG(Info, All, "Generated Asset: {}", job->productPath);
		}

		if (job->success)
		{
			AssetManager::GetRegistry()->OnAssetImported(job->productPath);
		}

		importResults.Add(importResult);

		numJobsInProgress--;
	}

	void AssetImportJob::BuildUuidTree(Ref<Bundle> bundle)
	{
    	rootNode.Clear();

    	std::function<void(AssetUuidNode*, Object*)> visitor =
    		[&](AssetUuidNode* parentNode, Object* object)
    		{
    			if (object == nullptr)
    				return;

    			if (parentNode != nullptr)
    			{
    				parentNode->children.Add({});
    			}
    			AssetUuidNode* node = nullptr;
    			if (parentNode != nullptr)
    			{
    				node = &parentNode->children.Top();
    			}
    			else
    			{
    				node = &rootNode;
    			}

    			node->uuid = object->GetUuid();
    			node->objectName = object->GetName();
    			node->objectType = object->GetClass();

			    for (int i = 0; i < object->GetSubObjectCount(); ++i)
			    {
				    visitor(node, object->GetSubObject(i));
			    }
    		};

    	visitor(nullptr, bundle.Get());
	}

	void AssetImportJob::FetchObjectUuids(const Ref<Bundle>& bundle, const Array<UuidFetcher>& uuidFetchers)
	{
		std::function<void(const Ref<Object>&, const Array<UuidFetcher>&)> visitor =
			[&](const Ref<Object>& object, const Array<UuidFetcher>& fetchers)
			{
				int count = Math::Min<int>(object->GetSubObjectCount(), fetchers.GetSize());

				for (int i = 0; i < count; ++i)
				{
					Ref<Object> subObject = object->GetSubObject(i);
					if (subObject->GetClass() == fetchers[i].clazz)
					{
						if (fetchers[i].outUuid != nullptr)
						{
							*fetchers[i].outUuid = subObject->GetUuid();
						}

						visitor(subObject, fetchers[i].children);
					}
				}
			};

		visitor(bundle.Get(), uuidFetchers);
	}

	Array<Name> AssetImportJob::PrepareProductAssetDependencies()
	{
		return {};
	}

	void AssetImportJob::Finish()
	{
		
	}

	void AssetImportJob::Process()
	{
		success = false;
		errorMessage = "";

		defer(&)
		{
			importer->OnAssetImportJobFinish(this);
		};

		if (!sourcePath.Exists() || sourcePath.IsDirectory())
		{
			errorMessage = "Source asset does not exist at path: " + sourcePath.GetString();
			return;
		}

		if (productPath.IsEmpty())
			productPath = sourcePath.ReplaceExtension(".casset");
		editorProductPath = sourcePath.ReplaceExtension(".casset");

		bundlePath = "";
		sourceAssetRelativePath = "";
		generateDistributionAsset = false;

		if (IO::Path::IsSubDirectory(sourcePath, gProjectPath / "Game/Assets"))
		{
			sourceAssetRelativePath = IO::Path::GetRelative(sourcePath, gProjectPath).GetString().Replace({ '\\' }, '/');
			if (!sourceAssetRelativePath.StartsWith("/"))
				sourceAssetRelativePath = "/" + sourceAssetRelativePath;
		}

		String assetName = productPath.GetFileName().RemoveExtension().GetString();
		if (!IsValidObjectName(assetName)) // Fix asset name
		{
			assetName = FixObjectName(assetName);
			productPath = productPath.GetParentPath() / (assetName + ".casset");
			editorProductPath = sourcePath.GetParentPath() / (assetName + ".casset");
		}

		auto engineRootDir = gProjectPath;
		if (!(engineRootDir / "Engine/Assets").Exists())
			engineRootDir = PlatformDirectories::GetAppRootDir();

		if (IO::Path::IsSubDirectory(productPath, gProjectPath / "Game/Assets"))
		{
			isGameAsset = true;
			bundlePath = IO::Path::GetRelative(productPath, gProjectPath).RemoveExtension().GetString().Replace({ '\\' }, '/');
			if (!bundlePath.StartsWith("/"))
				bundlePath = "/" + bundlePath;
		}
		else if (IO::Path::IsSubDirectory(productPath, engineRootDir / "Engine/Assets") || IO::Path::IsSubDirectory(productPath, engineRootDir / "Editor/Assets"))
		{
			isGameAsset = false;
			bundlePath = IO::Path::GetRelative(productPath, engineRootDir).RemoveExtension().GetString().Replace({ '\\' }, '/');
			if (!bundlePath.StartsWith("/"))
				bundlePath = "/" + bundlePath;
		}
		else
		{
			bundlePath = "/" + assetName;
		}
		
		if (editorProductPath != productPath)
		{
			// We are generating an asset that is NOT dependent on the source asset, ex: An asset to be used by standalone dist build.
			sourceAssetRelativePath = "";
			generateDistributionAsset = true;
		}

#if PLATFORM_WINDOWS
		// Replace backward slashes with forward ones
		sourcePath = sourcePath.GetString().Replace({ '\\' }, '/');
		editorProductPath = editorProductPath.GetString().Replace({ '\\' }, '/');
		productPath = productPath.GetString().Replace({'\\'}, '/');
#endif
		
		Ref<Bundle> bundle = nullptr;
    	Ref<Object> transient = CreateObject<Object>(nullptr, "Transient", OF_Transient);

    	rootNode.Clear();

		if (productPath.Exists())
		{
			LoadBundleArgs args{
				.loadFully = true,
				.forceReload = false,
				.destroyOutdatedObjects = true
			};

			bundle = Bundle::LoadBundleAbsolute(transient, productPath, args);

			if (bundle == nullptr)
			{
				IO::Path::Remove(productPath);
			}
			else // Asset was already processed before, but it requires an update now!
			{
				BuildUuidTree(bundle);
			}
		}

    	if (bundle == nullptr)
    	{
    		bundle = CreateObject<Bundle>(transient.Get(), assetName);
    	}
    	if (bundle == nullptr)
    	{
    		CE_LOG(Error, All, "Failed to create Bundle object!");
    		success = false;
    		return;
    	}

    	bundle->bundlePath = bundlePath;

		success = ProcessAsset(bundle);

		if (!success)
		{
			if (bundle)
				bundle->BeginDestroy();
			return;
		}

		if (!isGameAsset)
		{
			// Non-game assets (i.e., Non-user assets) must have a fixed UUID based on their path.
			// Because they are generated locally on each user's PC when the engine is built.
			// So we need to make sure that anyone who builds the engine has the same UUID for engine & editor assets.

			HashMap<Name, int> objectPathNameCounter{};

			std::function<void(Object*)> visitorFunc = [&](Object* object)
				{
					if (!object)
						return;

					if (object->IsOfType<Asset>())
					{
						Asset* asset = static_cast<Asset*>(object);
						asset->GetClass()->FindField("sourceAssetRelativePath")->SetFieldValue(asset, sourceAssetRelativePath);
					}

					String pathInBundle = object->GetPathInBundle().GetString();
					if (pathInBundle.NotEmpty())
						pathInBundle = "." + pathInBundle;
					String fullObjectPath = bundle->GetBundlePath().GetString() + pathInBundle;
					if (objectPathNameCounter[fullObjectPath] > 0)
					{
						fullObjectPath = String::Format(fullObjectPath + "_{}", objectPathNameCounter[fullObjectPath]);
					}

					Hash128 hash = CalculateHash128(fullObjectPath.GetCString(), fullObjectPath.GetLength());
					bundle->SetObjectUuid(object, Uuid(hash));

					objectPathNameCounter[fullObjectPath]++;

					for (int i = 0; i < object->GetSubObjectCount(); ++i)
					{
						Object* subObject = object->GetSubObject(i);
						visitorFunc(subObject);
					}
				};

			visitorFunc(bundle.Get());
		}

		auto saveResult = Bundle::SaveToDisk(bundle, nullptr, productPath);

    	// If this job created the bundle
    	if (bundle->GetOuter() == transient)
    	{
    		bundle->BeginDestroy();
    		bundle = nullptr;
    	}

		if (saveResult != BundleSaveResult::Success)
		{
			errorMessage = "Failed to save bundle " + assetName + " at path: " + productPath.GetString() + ". Error " + (int)saveResult;
			success = false;
			return;
		}
	}
    
} // namespace CE::Editor
