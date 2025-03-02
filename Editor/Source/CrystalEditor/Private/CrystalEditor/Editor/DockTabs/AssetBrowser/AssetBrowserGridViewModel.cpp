#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserGridViewModel::AssetBrowserGridViewModel()
    {

    }

    AssetBrowserGridViewModel::~AssetBrowserGridViewModel()
    {
        
    }

    void AssetBrowserGridViewModel::OnAssetPathTreeUpdated(PathTree& pathTree)
    {
        // Update the PathTreeNode* pointer in case the path tree nodes were recreated
        SetCurrentDirectory(currentPath);


    }

    void AssetBrowserGridViewModel::Init()
    {
        AssetManager* assetManager = AssetManager::Get();
        if (!assetManager)
            return;

        AssetRegistry* registry = assetManager->GetRegistry();
        if (!registry)
            return;

        registry->AddRegistryListener(this);

        currentDirectory = registry->GetCachedPathTree().GetRootNode();
    }

    void AssetBrowserGridViewModel::OnBeforeDestroy()
    {
        Super::OnBeforeDestroy();

        AssetManager* assetManager = AssetManager::Get();
        if (!assetManager)
            return;

        AssetRegistry* registry = assetManager->GetRegistry();
        if (!registry)
            return;

        registry->RemoveRegistryListener(this);
    }

    void AssetBrowserGridViewModel::SetCurrentDirectory(const Name& path)
    {
        currentPath = path;

        AssetManager* assetManager = AssetManager::Get();
        if (!assetManager)
            return;

        AssetRegistry* registry = assetManager->GetRegistry();
        if (!registry)
            return;

        currentDirectory = registry->GetCachedPathTree().GetNode(currentPath);
    }

    void AssetBrowserGridViewModel::SetData(AssetBrowserItem& item, PathTreeNode* node)
    {
        if (!node)
            return;

        item.SetData(node);
    }


} // namespace CE

