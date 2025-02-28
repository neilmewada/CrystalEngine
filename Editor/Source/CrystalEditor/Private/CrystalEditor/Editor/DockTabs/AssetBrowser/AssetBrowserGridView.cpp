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

    void AssetBrowserGridView::OnModelUpdate()
    {
        DestroyAllChildren();
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
                    .OnSelect([this](FSelectableButton* selectedItem)
                    {

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

