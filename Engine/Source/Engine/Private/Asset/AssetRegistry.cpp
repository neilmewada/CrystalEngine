#include "Engine.h"

namespace CE
{
	AssetRegistry* AssetRegistry::singleton = nullptr;

	static bool SortAssetData(AssetData* lhs, AssetData* rhs)
	{
		return String::NaturalCompare(lhs->bundleName.GetLastComponent(), rhs->bundleName.GetLastComponent());
	}

	AssetRegistry::AssetRegistry()
	{

	}

	AssetRegistry::~AssetRegistry()
	{
#if PAL_TRAIT_BUILD_EDITOR
		fileWatcher.RemoveWatcher(fileWatchID);
#endif
        
		for (auto assetData : allAssetDatas)
		{
			delete assetData;
		}
		allAssetDatas.Clear();

		cachedAssetsByPath.Clear();
	}

	void AssetRegistry::Shutdown()
	{
		
	}

	AssetRegistry* AssetRegistry::Get()
	{
		return AssetManager::GetRegistry();
	}

	void AssetRegistry::Refresh()
	{

	}

	AssetData* AssetRegistry::GetPrimaryAssetByPath(const Name& path)
	{
		return cachedPrimaryAssetByPath[path];
	}

	Array<AssetData*> AssetRegistry::GetAssetsByPath(const Name& path)
	{
		return cachedAssetsByPath[path];
	}

	AssetData* AssetRegistry::GetAssetBySourcePath(const Name& sourcePath)
	{
		if (cachedAssetBySourcePath.KeyExists(sourcePath))
			return cachedAssetBySourcePath[sourcePath];
		return nullptr;
	}

	Array<AssetData*> AssetRegistry::GetPrimaryAssetsInSubPath(const Name& parentPath)
	{
		return cachedPrimaryAssetsByParentPath[parentPath];
	}

	Array<String> AssetRegistry::GetSubDirectoriesAtPath(const Name& path)
	{
		Array<String> result{};

		auto directoryNode = cachedDirectoryTree.GetNode(path);
		if (directoryNode == nullptr)
			return result;

		for (const auto subDirectory : directoryNode->children)
		{
			result.Add(subDirectory->name.GetString());
		}

		return result;
	}

	PathTreeNode* AssetRegistry::GetDirectoryNode(const Name& path)
	{
		return cachedDirectoryTree.GetNode(path);
	}

	Name AssetRegistry::ResolveBundlePath(const Uuid& bundleUuid)
	{
		if (!cachedPrimaryAssetByBundleUuid.KeyExists(bundleUuid))
			return Name();
		AssetData* assetData = cachedPrimaryAssetByBundleUuid[bundleUuid];
		if (assetData == nullptr)
			return Name();
		return assetData->bundleName;
	}

	void AssetRegistry::AddRegistryListener(IAssetRegistryListener* listener)
	{
		listeners.Add(listener);
	}

	void AssetRegistry::RemoveRegistryListener(IAssetRegistryListener* listener)
	{
		listeners.Remove(listener);
	}


	void AssetRegistry::OnAssetImported(const IO::Path& bundleAbsolutePath, const Name& sourcePath)
	{
		LoadBundleArgs args{
			.loadFully = false,
			.forceReload = false,
			.destroyOutdatedObjects = false
		};

		Ref<Bundle> load = Bundle::LoadBundleAbsolute(nullptr, bundleAbsolutePath, args);
		if (load == nullptr)
			return;

		auto projectAssetsPath = gProjectPath / "Game/Assets";
		String relativePathStr = "";
		String parentRelativePathStr = "";

		auto engineInstallDir = EngineDirectories::GetEngineInstallDirectory();
		auto engineAssetsPath = engineInstallDir / "Engine/Assets";
		auto editorAssetsPath = engineInstallDir / "Editor/Assets";

		if (IO::Path::IsSubDirectory(bundleAbsolutePath, projectAssetsPath))
		{
			relativePathStr = IO::Path::GetRelative(bundleAbsolutePath, gProjectPath).RemoveExtension().GetString().Replace({'\\'}, '/');
			if (!relativePathStr.StartsWith("/"))
				relativePathStr = "/" + relativePathStr;

			parentRelativePathStr = IO::Path::GetRelative(bundleAbsolutePath, gProjectPath).GetParentPath().GetString().Replace({ '\\' }, '/');
			if (!parentRelativePathStr.StartsWith("/"))
				parentRelativePathStr = "/" + parentRelativePathStr;
		}
		else if (IO::Path::IsSubDirectory(bundleAbsolutePath, engineAssetsPath) || IO::Path::IsSubDirectory(bundleAbsolutePath, editorAssetsPath))
		{
			relativePathStr = IO::Path::GetRelative(bundleAbsolutePath, engineInstallDir).RemoveExtension().GetString().Replace({ '\\' }, '/');
			if (!relativePathStr.StartsWith("/"))
				relativePathStr = "/" + relativePathStr;

			parentRelativePathStr = IO::Path::GetRelative(bundleAbsolutePath, engineInstallDir).GetParentPath().GetString().Replace({ '\\' }, '/');
			if (!parentRelativePathStr.StartsWith("/"))
				parentRelativePathStr = "/" + parentRelativePathStr;
		}

		AssetData* assetData = nullptr;
		bool newEntry = false;
		int originalIndex = cachedPrimaryAssetsByParentPath[parentRelativePathStr]
			.IndexOf([&](AssetData* data) -> bool { return data->bundleName == load->GetName(); });

		if (originalIndex >= 0)
		{
			assetData = cachedPrimaryAssetsByParentPath[parentRelativePathStr].At(originalIndex);
		}

		if (assetData == nullptr)
		{
			assetData = new AssetData();
			newEntry = true;
		}

		String sourceAssetRelativePath = "";
		Bundle::ObjectData primaryObjectData = load->GetPrimaryObjectData();

		Name primaryName = primaryObjectData.name;
		Name primaryTypeName = primaryObjectData.typeName;
		assetData->bundleName = load->GetName();
		assetData->assetName = primaryName;
		assetData->assetClassPath = primaryTypeName;
		assetData->bundleUuid = load->GetUuid();
		assetData->assetUuid = primaryObjectData.uuid;
		
#if PAL_TRAIT_BUILD_EDITOR
		// Source asset path relative to project assets directory
		// TODO: Add source asset path
		//sourceAssetRelativePath = load->GetPrimarySourceAssetRelativePath();
		assetData->sourceAssetPath = sourceAssetRelativePath;
#endif
		
		if (newEntry && relativePathStr.NotEmpty())
		{
			AddAssetEntry(relativePathStr, assetData);
		}

		Name bundleName = assetData->bundleName;

		load->BeginDestroy();
		load = nullptr;

		for (IAssetRegistryListener* listener : listeners)
		{
			if (listener != nullptr)
			{
				listener->OnAssetImported(bundleName, sourcePath);
				listener->OnAssetPathTreeUpdated(cachedPathTree);
			}
		}
		
		onAssetImported.Broadcast(bundleName);
		onAssetRegistryModified.Broadcast();
	}

	void AssetRegistry::OnAssetDeleted(const Name& bundleName)
	{
		IO::Path bundlePath = Bundle::GetAbsoluteBundlePath(bundleName);

		auto projectAssetsPath = gProjectPath / "Game/Assets";
		String relativePathStr = "";
		String parentRelativePathStr = "";

		if (IO::Path::IsSubDirectory(bundlePath, projectAssetsPath))
		{
			relativePathStr = IO::Path::GetRelative(bundlePath, gProjectPath).RemoveExtension().GetString().Replace({ '\\' }, '/');
			if (!relativePathStr.StartsWith("/"))
				relativePathStr = "/" + relativePathStr;

			parentRelativePathStr = IO::Path::GetRelative(bundlePath, gProjectPath).GetParentPath().GetString().Replace({ '\\' }, '/');
			if (!parentRelativePathStr.StartsWith("/"))
				parentRelativePathStr = "/" + parentRelativePathStr;
		}
		else
		{
			return;
		}

		Name parentRelativePathName = parentRelativePathStr;
		Name relativePathName = relativePathStr;

		cachedPrimaryAssetsByParentPath.Remove(parentRelativePathName);

		cachedPathTree.RemovePath(relativePathName);
		cachedAssetsByPath.Remove(relativePathName);

		for (IAssetRegistryListener* listener : listeners)
		{
			if (listener != nullptr)
			{
				listener->OnAssetImported(bundleName);
				listener->OnAssetPathTreeUpdated(cachedPathTree);
			}
		}

		onAssetRegistryModified.Broadcast();
	}

	void AssetRegistry::InitializeCache()
	{
		if (cacheInitialized)
			return;

		// Clear the path tree
		cachedPathTree.RemoveAll();
		cachedDirectoryTree.RemoveAll();

		cachedPathTree.AddPath("/Game"); cachedDirectoryTree.AddPath("/Game");
		cachedPathTree.AddPath("/Engine"); cachedDirectoryTree.AddPath("/Engine");
#if PAL_TRAIT_BUILD_EDITOR
		//pathTree.AddPath("/Editor"); directoryTree.AddPath("/Editor");
#endif
		
		// Game assets
		if (gProjectPath.Exists() && (gProjectPath / "Game/Assets").Exists())
		{
			auto projectAssetsPath = gProjectPath / "Game/Assets";
			
			projectAssetsPath.RecursivelyIterateChildren([&](const IO::Path& item)
				{
					auto relativePath = IO::Path::GetRelative(item, gProjectPath);
					auto relativePathStr = relativePath.RemoveExtension().GetString().Replace({'\\'}, '/');
                    if (!relativePathStr.StartsWith("/"))
						relativePathStr = "/" + relativePathStr;

					String parentRelativePathStr = relativePath.GetParentPath().GetString().Replace({ '\\' }, '/');
					if (!parentRelativePathStr.StartsWith("/"))
						parentRelativePathStr = "/" + parentRelativePathStr;

					if (item.IsDirectory()) // Folder
					{
                        if (!relativePathStr.IsEmpty())
						{
							cachedPathTree.AddPath(relativePathStr);
							cachedDirectoryTree.AddPath(relativePathStr);
						}
					}
                    else if (item.GetExtension() == ".casset") // Product asset file
                    {
                    	LoadBundleArgs args{
                    		.loadFully = false,
                    		.forceReload = false,
                    		.destroyOutdatedObjects = false
                    	};

						Ref<Bundle> load = Bundle::LoadBundleAbsolute(nullptr, item, args);
						if (load != nullptr)
						{
							AssetData* assetData = new AssetData();
							auto primaryObjectData = load->GetPrimaryObjectData();
							//if (!load->GetPrimaryObjectName().IsValid())
							//	load->LoadFully();
							Name primaryName = primaryObjectData.name;
							Name primaryTypeName = primaryObjectData.typeName;
							assetData->bundleName = load->GetName();
							assetData->assetName = primaryName;
							assetData->assetClassPath = primaryTypeName;
							assetData->bundleUuid = load->GetUuid();
							assetData->assetUuid = primaryObjectData.uuid;
#if PAL_TRAIT_BUILD_EDITOR
							// Source asset path relative to project assets directory
							// TODO: Get source asset relative path
							//assetData->sourceAssetPath = load->GetPrimarySourceAssetRelativePath();
#endif
							load->BeginDestroy();
							load = nullptr;

							AddAssetEntry(relativePathStr, assetData);
						}
						else
						{
							CE_LOG(Error, All, "Failed to load asset metadata: {}", item);
						}
                    }
				});

#if PAL_TRAIT_BUILD_EDITOR
			fileWatchID = fileWatcher.AddWatcher(projectAssetsPath, this, true);
			fileWatcher.Watch();
#endif
		}

		auto engineDir = gProjectPath;

		if (!gProjectPath.Exists() || !(gProjectPath / "Engine/Assets").Exists())
		{
			engineDir = PlatformDirectories::GetEngineRootDir();
		}

		// Engine assets
		if (engineDir.Exists() && (engineDir / "Engine/Assets").Exists())
		{
			auto projectAssetsPath = engineDir / "Engine/Assets";

			projectAssetsPath.RecursivelyIterateChildren([&](const IO::Path& item)
				{
					auto relativePath = IO::Path::GetRelative(item, engineDir);
					auto relativePathStr = relativePath.RemoveExtension().GetString().Replace({ '\\' }, '/');
					if (!relativePathStr.StartsWith("/"))
						relativePathStr = "/" + relativePathStr;

					String parentRelativePathStr = relativePath.GetParentPath().GetString().Replace({ '\\' }, '/');
					if (!parentRelativePathStr.StartsWith("/"))
						parentRelativePathStr = "/" + parentRelativePathStr;

					if (item.IsDirectory()) // Folder
					{
						if (!relativePathStr.IsEmpty())
						{
							cachedPathTree.AddPath(relativePathStr);
							cachedDirectoryTree.AddPath(relativePathStr);
						}
					}
					else if (item.GetExtension() == ".casset") // Product asset file
					{
						LoadBundleArgs args{
							.loadFully = false
						};

						Ref<Bundle> load = Bundle::LoadBundleAbsolute(nullptr, item, args);
						if (load != nullptr)
						{
							AssetData* assetData = new AssetData();
							auto primaryObjectData = load->GetPrimaryObjectData();

							Name primaryName = primaryObjectData.name;
							Name primaryTypeName = primaryObjectData.typeName;
							assetData->bundleName = load->GetName();
							assetData->assetName = primaryName;
							assetData->assetClassPath = primaryTypeName;
							assetData->bundleUuid = load->GetUuid();
							assetData->assetUuid = primaryObjectData.uuid;
#if PAL_TRAIT_BUILD_EDITOR
							// Source asset path relative to project assets directory
							//assetData->sourceAssetPath = load->GetPrimarySourceAssetRelativePath();
#endif
							load->BeginDestroy();
							load = nullptr;

							AddAssetEntry(relativePathStr, assetData);
						}
						else
						{
							CE_LOG(Error, All, "Failed to load asset metadata: {}", item);
						}
					}
				});
		}

		auto launchDir = PlatformDirectories::GetLaunchDir();

		// Editor assets
		if ((launchDir / "Editor/Assets").Exists())
		{
			auto editorAssetsPath = launchDir / "Editor/Assets";

			editorAssetsPath.RecursivelyIterateChildren([&](const IO::Path& item)
				{
					auto relativePath = IO::Path::GetRelative(item, launchDir);
					auto relativePathStr = relativePath.RemoveExtension().GetString().Replace({ '\\' }, '/');
					if (!relativePathStr.StartsWith("/"))
						relativePathStr = "/" + relativePathStr;

					String parentRelativePathStr = relativePath.GetParentPath().GetString().Replace({ '\\' }, '/');
					if (!parentRelativePathStr.StartsWith("/"))
						parentRelativePathStr = "/" + parentRelativePathStr;

					if (item.IsDirectory()) // Folder
					{
						if (!relativePathStr.IsEmpty())
						{
							cachedPathTree.AddPath(relativePathStr);
							cachedDirectoryTree.AddPath(relativePathStr);
						}
					}
					else if (item.GetExtension() == ".casset") // Product asset file
					{
						LoadBundleArgs args{
							.loadFully = false
						};

						Ref<Bundle> load = Bundle::LoadBundleAbsolute(nullptr, item, args);
						if (load != nullptr)
						{
							AssetData* assetData = new AssetData();
							auto primaryObjectData = load->GetPrimaryObjectData();

							Name primaryName = primaryObjectData.name;
							Name primaryTypeName = primaryObjectData.typeName;
							assetData->bundleName = load->GetName();
							assetData->assetName = primaryName;
							assetData->assetClassPath = primaryTypeName;
							assetData->bundleUuid = load->GetUuid();
							assetData->assetUuid = primaryObjectData.uuid;
#if PAL_TRAIT_BUILD_EDITOR
							// Source asset path relative to project assets directory
							// TODO:
							//assetData->sourceAssetPath = load->GetPrimarySourceAssetRelativePath();
#endif
							load->BeginDestroy();
							load = nullptr;

							AddAssetEntry(relativePathStr, assetData);
						}
						else
						{
							CE_LOG(Error, All, "Failed to load asset metadata: {}", item);
						}
					}
				}
			);
		}

		cacheInitialized = true;
	}


	void AssetRegistry::AddAssetEntry(const Name& bundleName, AssetData* assetData)
	{
		if (assetData == nullptr)
			return;

		allAssetDatas.Add(assetData);
		cachedPathTree.AddPath(bundleName, PathTreeNodeType::Asset, assetData);
		cachedAssetsByPath[bundleName].Add(assetData);
		cachedPrimaryAssetByPath[bundleName] = assetData;
		cachedPrimaryAssetByBundleUuid[assetData->bundleUuid] = assetData;

		String parentPathStr = IO::Path(bundleName.GetString()).GetParentPath().GetString().Replace({ '\\' }, '/');
		Name parentPath = parentPathStr;
		
		cachedPrimaryAssetsByParentPath[parentPath].Add(assetData);
		cachedPrimaryAssetsByParentPath[parentPath].Sort(SortAssetData);

		if (assetData->sourceAssetPath.IsValid())
		{
			cachedAssetBySourcePath[assetData->sourceAssetPath] = assetData;
		}


	}

	void AssetRegistry::DeleteAssetEntry(const Name& bundleName)
	{
		if (!cachedAssetsByPath.KeyExists(bundleName))
			return;

		auto assetDatas = cachedAssetsByPath[bundleName];

		cachedPathTree.RemovePath(bundleName);
		cachedAssetsByPath.Remove(bundleName);
		cachedPrimaryAssetByPath.Remove(bundleName);
		
		String parentPathStr = IO::Path(bundleName.GetString()).GetParentPath().GetString().Replace({ '\\' }, '/');
		Name parentPath = parentPathStr;

		cachedPrimaryAssetsByParentPath.Remove(parentPath);

		for (auto assetData : assetDatas)
		{
			if (assetData != nullptr)
			{
				Name sourcePath = assetData->sourceAssetPath;
				if (cachedAssetBySourcePath.KeyExists(sourcePath))
				{
					cachedAssetBySourcePath.Remove(sourcePath);
				}
				delete assetData;
			}
		}
	}

	void AssetRegistry::HandleFileAction(IO::WatchID watchId, IO::Path directory, const String& fileName, IO::FileAction fileAction, const String& oldFileName)
	{
		mutex.Lock();

		// Watch for new/modified source assets
		IO::Path relative = IO::Path::GetRelative(directory, gProjectPath / "Game/Assets").GetString().Replace({ '\\' }, '/');
		u64 length = 0;
		auto filePath = directory / fileName;
        bool isDirectory = filePath.IsDirectory();
        bool isFile = !isDirectory;

		if (filePath.Exists() && !isDirectory)
		{
			FileStream stream = FileStream(directory / fileName, Stream::Permissions::ReadOnly);
			stream.SetBinaryMode(true);

			length = stream.GetLength();

			stream.Close();
		}
        
		if (isFile && (length > 0 || fileAction == IO::FileAction::Delete || fileAction == IO::FileAction::Moved || fileAction == IO::FileAction::Add))
		{
            if (length > 0 && (fileAction == IO::FileAction::Modified || fileAction == IO::FileAction::Add))
            {
				if (sourceChanges.IsEmpty() || 
					//sourceChanges.Top().fileAction != IO::FileAction::Modified || 
					sourceChanges.Top().currentPath != filePath)
				{
					SourceAssetChange change{};
					change.fileAction = IO::FileAction::Modified;
					change.fileSize = length;
					change.currentPath = filePath;
					change.oldPath = "";
					sourceChanges.Add(change);
				}
            }
            else if (fileAction == IO::FileAction::Delete)
            {
                SourceAssetChange change{};
                change.fileAction = IO::FileAction::Delete;
                change.currentPath = filePath;
                change.fileSize = length;
                sourceChanges.Add(change);
            }
            else if (fileAction == IO::FileAction::Moved)
            {
                SourceAssetChange change{};
                change.fileAction = IO::FileAction::Moved;
                change.currentPath = filePath;
                change.oldPath = directory / oldFileName;
                change.fileSize = length;
                sourceChanges.Add(change);
            }
		}

		mutex.Unlock();

		// FIX: Added delay to prevent skipping file Modified calls
		Thread::SleepFor(1);
	}

} // namespace CE
