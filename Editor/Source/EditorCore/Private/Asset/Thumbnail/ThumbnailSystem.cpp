#include "EditorCore.h"

namespace CE
{
	static WeakRef<ThumbnailSystem> instance = nullptr;

    ThumbnailSystem::ThumbnailSystem()
    {
        if (!IsDefaultInstance())
        {
            instance = this;
        }
    }

    void ThumbnailSystem::OnBeforeDestroy()
    {
	    Super::OnBeforeDestroy();

        if (instance == this)
        {
			instance = nullptr;
        }
    }

    void ThumbnailSystem::OnAssetImported(const Name& bundlePath, const Name& sourcePath)
    {
        MarkAssetDirty(bundlePath);
    }

    void ThumbnailSystem::OnAssetRenamed(Uuid bundleUuid, const Name& oldName, const Name& newName, const Name& newPath)
    {
        MarkAssetDirty(newPath);
    }

    void ThumbnailSystem::MarkAssetDirty(const Name& bundlePath)
    {
        AssetData* assetData = AssetManager::Get()->GetPrimaryAssetDataAtPath(bundlePath);
        if (!assetData)
            return;

        SubClass<Asset> clazz = ClassType::FindClass(assetData->assetClassTypeName);
        if (!clazz)
            return;

        Ref<AssetDefinition> assetDef = AssetDefinitionRegistry::Get()->FindAssetDefinition(clazz);
        if (!assetDef)
            return;

        SubClass<AssetThumbnailGen> thumbnailGenClass = assetDef->GetThumbnailGeneratorClass();
        if (!thumbnailGenClass)
            return;

        AssetThumbnailGen* defaultInstance = (AssetThumbnailGen*)thumbnailGenClass->GetDefaultInstance();
        if (!defaultInstance->IsValidForAssetType(clazz))
            return;

        Name assetPath = assetData->bundlePath;

        dirtyAssetsByThumbnailGenClass[thumbnailGenClass->GetTypeId()].Add(assetPath);
    }

    ThumbnailSystem::~ThumbnailSystem()
    {
        
    }

    Ref<ThumbnailSystem> ThumbnailSystem::Get()
    {
        return instance.Lock();
    }

    void ThumbnailSystem::Initialize()
    {
        AssetRegistry* registry = AssetManager::GetRegistry();
        if (!registry)
            return;

        registry->AddRegistryListener(this);

    	registry->GetCachedPathTree().IterateNodesRecursively([&](PathTreeNode* node)
    	{
            if (node == nullptr)
                return;

            if (node->nodeType == PathTreeNodeType::Asset)
            {
	            if (AssetData* assetData = (AssetData*)node->userData)
                {
                    SubClass<Asset> clazz = ClassType::FindClass(assetData->assetClassTypeName);
                    if (!clazz)
						return;

					Ref<AssetDefinition> assetDef = AssetDefinitionRegistry::Get()->FindAssetDefinition(clazz);
					if (!assetDef)
                        return;

                    SubClass<AssetThumbnailGen> thumbnailGenClass = assetDef->GetThumbnailGeneratorClass();
                    if (!thumbnailGenClass)
						return;

                    AssetThumbnailGen* defaultInstance = (AssetThumbnailGen*)thumbnailGenClass->GetDefaultInstance();
                    if (!defaultInstance->IsValidForAssetType(clazz))
						return;

                    Name assetPath = assetData->bundlePath;
					Name thumbnailPath = AssetThumbnailGen::GetThumbnailPath(assetPath);
					IO::Path absoluteThumbnailPath = Bundle::GetAbsoluteBundlePath(thumbnailPath);
                    if (absoluteThumbnailPath.Exists())
                    {
	                    return;
                    }

                    dirtyAssetsByThumbnailGenClass[thumbnailGenClass->GetTypeId()].Add(assetPath);
                }
            }
    	});
    }

    void ThumbnailSystem::Shutdown()
    {
        AssetRegistry* registry = AssetManager::GetRegistry();
        if (!registry)
            return;


		registry->RemoveRegistryListener(this);
    }

    void ThumbnailSystem::Tick(f32 deltaTime)
    {
        if (dirtyAssetsByThumbnailGenClass.IsEmpty())
            return;

        // Process dirty assets and generate thumbnails
        for (const auto& [thumbnailClassId, assetPaths] : dirtyAssetsByThumbnailGenClass)
        {
            SubClass<AssetThumbnailGen> thumbnailGenClass = ClassType::FindClass(thumbnailClassId);
            if (!thumbnailGenClass)
                continue;

            Ref<AssetThumbnailGen> thumbnailGen = CreateObject<AssetThumbnailGen>(this, thumbnailGenClass->GetName().GetLastComponent(), OF_NoFlags, thumbnailGenClass);
            thumbnailGenerators.Add(thumbnailGen);

            thumbnailGen->SetAssetPaths(assetPaths);
            thumbnailGen->onFinishEvent += [this](Ref<AssetThumbnailGen> thumbnailGen)
                {
                    OnThumbnailFinished(thumbnailGen);
                };

			thumbnailGen->StartProcessing();
        }

        // Clear the dirty assets after processing
        dirtyAssetsByThumbnailGenClass.Clear();
    }

    void ThumbnailSystem::AddThumbnailListener(IThumbnailSystemListener* listener)
    {
        if (!listener)
            return;

        if (!thumbnailListeners.Exists(listener))
        {
            thumbnailListeners.Add(listener);
		}
    }

    void ThumbnailSystem::RemoveThumbnailListener(IThumbnailSystemListener* listener)
    {
        thumbnailListeners.Remove(listener);
    }

    Name ThumbnailSystem::GetThumbnailPath(Name assetPath)
    {
        return "/Temp/ThumbnailCache" + assetPath.GetString();
    }

    void ThumbnailSystem::OnThumbnailFinished(Ref<AssetThumbnailGen> thumbnailGen)
    {
        if (!thumbnailGen)
			return;

        const Array<Name>& assetPaths = thumbnailGen->GetAssetPaths();

        for (IThumbnailSystemListener* thumbnailListener : thumbnailListeners)
        {
			thumbnailListener->OnThumbnailsUpdated(assetPaths);
        }

        thumbnailGenerators.Remove(thumbnailGen);
        thumbnailGen->BeginDestroy();
    }
} // namespace CE

