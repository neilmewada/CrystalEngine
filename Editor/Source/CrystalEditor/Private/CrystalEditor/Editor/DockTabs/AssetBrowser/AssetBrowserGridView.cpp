#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserGridView::AssetBrowserGridView()
    {
        
    }

    void AssetBrowserGridView::Construct()
    {
        Super::Construct();


    }

    void AssetBrowserGridView::DeselectAll()
    {
        selectedItems.Clear();
        OnSelectionUpdated();
    }

    void AssetBrowserGridView::OnModelUpdate()
    {
        QueueDestroyAllChildren();
        items.Clear();

        if (!m_Model)
            return;

        PathTreeNode* currentDirectory = m_Model->GetCurrentDirectory();

        if (currentDirectory != nullptr)
        {
            for (int i = 0; i < currentDirectory->children.GetSize(); i++)
            {
                AssetBrowserItem* item = nullptr;

                AddChild(
                    FAssignNew(AssetBrowserItem, item)
                    .Owner(this)
                    .OnDoubleClick([this](FSelectableButton* button)
                    {
                        if (auto registry = AssetRegistry::Get())
                        {
                            PathTree& tree = registry->GetCachedPathTree();
                            AssetBrowserItem* item = static_cast<AssetBrowserItem*>(button);
                            CE::Name path = item->GetFullPath();
                            PathTreeNode* curNode = tree.GetNode(path);
                            if (curNode == nullptr)
                                return;

                            if (auto owner = m_Owner.Lock())
                            {
                                if (curNode->nodeType == PathTreeNodeType::Directory)
                                {
                                    owner->SetCurrentPath(path);
                                }
                                else if (curNode->nodeType == PathTreeNodeType::Asset)
                                {
                                    // Do nothing when double-clicking an asset
                                }
                            }
                        }
                    })
                );

                items.Add(item);

                m_Model->SetData(*item, currentDirectory->children[i]);
            }
        }
    }

    void AssetBrowserGridView::OnSelectionUpdated()
    {
        for (AssetBrowserItem* item : items)
        {
            if (selectedItems.Exists(item))
            {
                item->Select();
            }
            else
            {
                item->Deselect();
            }
        }
    }

    void AssetBrowserGridView::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            auto mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->sender == this && mouseEvent->buttons == MouseButtonMask::Left &&
                mouseEvent->type == FEventType::MousePress)
            {
                mouseEvent->Consume(this);

                selectedItems.Clear();
                OnSelectionUpdated();
            }
        }

        Super::HandleEvent(event);
    }

    
}

