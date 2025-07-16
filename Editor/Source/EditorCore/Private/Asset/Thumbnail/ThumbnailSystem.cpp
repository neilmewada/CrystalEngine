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
            thumbnailGen->onFinishEvent += [thumbnailClassId, this](Ref<AssetThumbnailGen> thumbnailGen)
                {
                    OnThumbnailFinished(thumbnailGen);
                };

			thumbnailGen->StartProcessing();
        }

        // Clear the dirty assets after processing
        dirtyAssetsByThumbnailGenClass.Clear();
    }

    void ThumbnailSystem::OnThumbnailFinished(Ref<AssetThumbnailGen> thumbnailGen)
    {
        if (!thumbnailGen)
			return;

        thumbnailGenerators.Remove(thumbnailGen);
        thumbnailGen->BeginDestroy();

        CE_LOG(Info, All, "Finished Processing: {}", thumbnailGen->GetClass()->GetName());
    }
} // namespace CE

