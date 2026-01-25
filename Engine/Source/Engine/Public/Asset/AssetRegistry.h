#pragma once

namespace CE
{
#if PAL_TRAIT_BUILD_EDITOR
	namespace Editor {
		class AssetImporter;
		class EditorAssetManager;
		class EditorAssetManager;
		class EditorBase;
	}
#endif

	class AssetRegistry;

	struct IAssetRegistryListener
	{
		virtual ~IAssetRegistryListener() {}

		virtual void OnAssetImported(const Name& bundlePath, const Name& sourcePath = "") {}

		virtual void OnAssetUnloaded(Uuid bundleUuid) {}

		virtual void OnAssetDeleted(const Name& bundlePath) {}

		virtual void OnAssetRenamed(Uuid bundleUuid, const Name& oldName, const Name& newName, const Name& newPath) {}

		virtual void OnDirectoryRenamed(const Name& oldPath, const Name& newPath) {}

		virtual void OnAssetPathTreeUpdated(PathTree& pathTree) {}

	};

	CLASS()
	class ENGINE_API AssetRegistry : public Object, IO::IFileWatchListener, public IBundleResolver
	{
		CE_CLASS(AssetRegistry, Object)
	public:

		AssetRegistry();
		virtual ~AssetRegistry();

		void Shutdown();

		static AssetRegistry* Get();

		void Refresh();

		PathTree& GetCachedDirectoryPathTree()
		{
			return cachedDirectoryTree;
		}

		PathTree& GetCachedPathTree()
		{
			return cachedPathTree;
		}

		struct SourceAssetChange
		{
			IO::FileAction fileAction{};
			IO::Path currentPath{};
			IO::Path oldPath{};
            u64 fileSize = 0;

			inline bool IsValid()
			{
				return !currentPath.IsEmpty() || !oldPath.IsEmpty();
			}
		};

		// - Asset Registry API -

		AssetData* GetPrimaryAssetByPath(const Name& path);
		Array<AssetData*> GetAssetsByPath(const Name& path);
		AssetData* GetAssetBySourcePath(const Name& sourcePath);

		Array<AssetData*> GetPrimaryAssetsInSubPath(const Name& parentPath);

		const Array<AssetData*>& GetAllAssetsOfType(TypeId typeId);

		Array<String> GetSubDirectoriesAtPath(const Name& path);
		PathTreeNode* GetDirectoryNode(const Name& path);
		
		Name ResolveBundlePath(const Uuid& uuid) override;

		void AddRegistryListener(IAssetRegistryListener* listener);
		void RemoveRegistryListener(IAssetRegistryListener* listener);

	protected:

		/// Caches path tree structure
		void InitializeCache();

		// Inherited via IFileWatchListener
		virtual void HandleFileAction(IO::WatchID watchId, IO::Path directory, const String& fileName, IO::FileAction fileAction, const String& oldFileName) override;

		// - Internal API -

		void OnAssetImported(const IO::Path& bundleAbsolutePath, const Name& sourcePath = "");

		void OnAssetUpdated(const Name& bundlePath);

		void OnDirectoryCreated(const IO::Path& absolutePath);

		void OnDirectoryRenamed(const Name& originalPath, const Name& newName);

		void OnDirectoryDeleted(const Name& directoryPath);

		void OnAssetRenamed(const Name& originalPath, const IO::Path& newAbsolutePath, const Name& newName);

		void OnDirectoryAndAssetsDeleted(const Array<Name>& paths);

	public:


	private:

		void AddAssetEntry(const Name& bundleName, AssetData* assetData);
		void DeleteAssetEntry(const Name& bundlePath);

		PathTree cachedDirectoryTree{};
		PathTree cachedPathTree{};

		b8 cacheInitialized = false;

		Mutex sourceChangesMutex{};
        Array<SourceAssetChange> sourceChanges{};

		static AssetRegistry* singleton;

		friend class AssetManager;

#if PAL_TRAIT_BUILD_EDITOR
		friend class CE::Editor::EditorAssetManager;
		friend class CE::Editor::AssetImporter;
		friend class CE::Editor::EditorAssetManager;
		friend class CE::Editor::EditorBase;
#endif
		IO::FileWatcher fileWatcher{};
		IO::WatchID fileWatchID = 0;

		Array<IAssetRegistryListener*> listeners;

		// Asset Registry State

		Array<AssetData*> allAssetDatas{};

		// - Caches -

		SharedRecursiveMutex cacheMutex;
		HashMap<Uuid, AssetData*> cachedPrimaryAssetByBundleUuid{};
		HashMap<Name, AssetData*> cachedPrimaryAssetByPath{};
		HashMap<Name, Array<AssetData*>> cachedAssetsByPath{};
		HashMap<Name, AssetData*> cachedAssetBySourcePath{};

		HashMap<TypeId, Array<AssetData*>> cachedAssetsByType{};

		// List of primary assets in the sub-path of a path
		HashMap<Name, Array<AssetData*>> cachedPrimaryAssetsByParentPath{};
		
	};
    
} // namespace CE

#include "AssetRegistry.rtti.h"
