#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserTreeViewModel::AssetBrowserTreeViewModel()
    {

    }

    AssetBrowserTreeViewModel::~AssetBrowserTreeViewModel()
    {
        
    }

    void AssetBrowserTreeViewModel::Init()
    {
        AssetManager* assetManager = AssetManager::Get();
        if (!assetManager)
            return;

        AssetRegistry* registry = assetManager->GetRegistry();
        if (!registry)
            return;

        
    }

    void AssetBrowserTreeViewModel::OnBeforeDestroy()
    {
        Super::OnBeforeDestroy();

        AssetManager* assetManager = AssetManager::Get();
        if (!assetManager)
            return;

        AssetRegistry* registry = assetManager->GetRegistry();
        if (!registry)
            return;


    }


    FModelIndex AssetBrowserTreeViewModel::GetParent(const FModelIndex& index)
    {
	    if (!index.IsValid() || index.GetDataPtr() == nullptr)
	        return FModelIndex();

        PathTreeNode* node = (PathTreeNode*)index.GetDataPtr();

        PathTreeNode* parent = node->parent;
        if (parent == nullptr)
            return FModelIndex();

        PathTreeNode* parentsParent = parent->parent;
        if (parentsParent == nullptr)
            return FModelIndex();

        int parentsIndex = parentsParent->children.IndexOf(parent);
        if (parentsIndex == -1)
            return FModelIndex();

        FModelIndex retIndex = CreateIndex(parentsIndex, 0, parent);
        return retIndex;
    }

    FModelIndex AssetBrowserTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        AssetRegistry* registry = AssetManager::Get()->GetRegistry();
        if (registry == nullptr)
            return {};

        PathTreeNode* parentNode = nullptr;

        if (!parent.IsValid())
        {
            parentNode = registry->GetCachedDirectoryPathTree().GetRootNode();
        }
        else
        {
            parentNode = (PathTreeNode*)parent.GetDataPtr();
        }

        if (parentNode == nullptr || row >= parentNode->children.GetSize())
            return {};

        FModelIndex index = CreateIndex(row, column, parentNode->children[row]);
        return index;
    }

    FModelIndex AssetBrowserTreeViewModel::FindIndex(PathTreeNode* node)
    {
        FModelIndex parent = {};
        FModelIndex result = {};

        Array<PathTreeNode*> paths;
        PathTreeNode* cur = node;

        while (cur != nullptr && cur->parent != nullptr)
        {
            paths.InsertAt(0, cur);
            cur = cur->parent;
        }

        for (int i = 0; i < paths.GetSize(); ++i)
        {
            int rows = GetRowCount(parent);
            bool found = false;

            for (int row = 0; row < rows; ++row)
            {
                FModelIndex index = GetIndex(row, 0, parent);
                PathTreeNode* indexNode = static_cast<PathTreeNode*>(index.GetDataPtr());
                if (indexNode->GetFullPath() == paths[i]->GetFullPath())
                {
                    found = true;
                    parent = index;
                    result = index;
                    break;
                }
            }

            if (!found)
                return {};
        }

        return result;
    }

    u32 AssetBrowserTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        AssetRegistry* registry = AssetManager::Get()->GetRegistry();
        if (registry == nullptr)
            return 0;

        PathTreeNode* parentNode = nullptr;

        if (!parent.IsValid())
        {
            AssetRegistry* registry = AssetManager::Get()->GetRegistry();

            parentNode = registry->GetCachedDirectoryPathTree().GetRootNode();
        }
        else
        {
            parentNode = (PathTreeNode*)parent.GetDataPtr();
        }

        if (parentNode == nullptr)
        {
            return 0;
        }

        return parentNode->children.GetSize();
    }

    u32 AssetBrowserTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return 1;
    }

    void AssetBrowserTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
    {
        FTreeViewRow* rowCast = Object::CastTo<FTreeViewRow>(&rowWidget);
        if (rowCast == nullptr)
            return;

        FTreeViewRow& treeRow = *rowCast;

        FModelIndex index = GetIndex(row, 0, parent);
        if (!index.IsValid() || index.GetDataPtr() == nullptr)
            return;

        PathTreeNode* node = (PathTreeNode*)index.GetDataPtr();

        FTreeViewCell& cell = *treeRow.GetCell(0);

        cell
            .Text(node->name.GetString())
			.IconEnabled(true)
            ;
    }

} // namespace CE

