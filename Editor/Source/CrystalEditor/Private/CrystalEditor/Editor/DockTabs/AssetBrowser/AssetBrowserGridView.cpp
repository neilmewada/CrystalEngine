#include "CrystalEditor.h"

namespace CE::Editor
{
    static constexpr f32 MinContextMenuWidth = 200;

    AssetBrowserGridView::AssetBrowserGridView()
    {
        contextMenu = CreateDefaultSubobject<EditorMenuPopup>("ContextMenu");
    }

    void AssetBrowserGridView::Construct()
    {
        Super::Construct();


    }

    void AssetBrowserGridView::DeselectAll()
    {
        for (AssetBrowserItem* item : items)
        {
            item->Deselect();
        }
    }

    void AssetBrowserGridView::SetCurrentDirectory(const CE::Name& directory)
    {
        this->currentPath = directory;

    }

    int AssetBrowserGridView::GetSelectedItemCount()
    {
        int count = 0;
        for (AssetBrowserItem* item : items)
        {
            if (item->IsActive())
                count++;
        }
        return count;
    }

    Array<AssetBrowserItem*> AssetBrowserGridView::GetSelectedItems()
    {
        Array<AssetBrowserItem*> selection;
        for (AssetBrowserItem* item : items)
        {
	        if (item->IsActive())
	        {
                selection.Add(item);
	        }
        }
        return selection;
    }

    void AssetBrowserGridView::SelectItem(const CE::Name& fullPath)
    {
        for (AssetBrowserItem* item : items)
        {
            if (item->GetFullPath() == fullPath)
            {
                item->Select();
                return;
            }
        }
    }

    void AssetBrowserGridView::OnUpdate()
    {
        QueueDestroyAllChildren();
        items.Clear();

        PathTreeNode* currentDirectory = AssetRegistry::Get()->GetCachedPathTree().GetNode(this->currentPath);

        if (currentDirectory != nullptr)
        {
            AssetBrowserItem* itemToRename = nullptr;
            AssetBrowserItem* selectedItem = nullptr;

            for (int i = 0; i < currentDirectory->children.GetSize(); i++)
            {
                AssetBrowserItem* item = nullptr;
                auto node = currentDirectory->children[i];
                if (node->parent != nullptr && node->parent->parent == nullptr && node->name == "Editor")
                {
                    continue;
                }

                AddChild(
                    FAssignNew(AssetBrowserItem, item)
                    .Owner(this)
                    .OnDoubleClick([this](FSelectableButton* button)
                    {
                        if (auto registry = AssetRegistry::Get())
                        {
                            PathTree& tree = registry->GetCachedPathTree();
                            AssetBrowserItem* item = static_cast<AssetBrowserItem*>(button);
                            if (item->IsEditing())
                                return;

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
                                    owner->OpenAsset(path);
                                }
                            }
                        }
                    })
                );

                items.Add(item);

                item->SetData(node);

                if (folderToRename.IsValid() && node->GetFullPath() == folderToRename.GetString())
                {
                    itemToRename = item;
                }

                if (itemToSelect.IsValid() && node->GetFullPath() == itemToSelect.GetString())
                {
                    selectedItem = item;
                    itemToSelect = nullptr;
                }
            }

            if (itemToRename != nullptr)
            {
                itemToRename->Select();
                StartRenaming();

                folderToRename = {};
            }

            if (selectedItem != nullptr)
            {
                selectedItem->Select();
            }
        }
    }

    void AssetBrowserGridView::OnSelectionUpdated()
    {
        for (AssetBrowserItem* item : items)
        {
            item->Deselect();
        }
    }

    void AssetBrowserGridView::OnBackgroundRightClicked(Vec2 globalMousePos)
    {
        if (!currentPath.IsValid())
            return;
        if (!currentPath.GetString().StartsWith("/Game/Assets"))
            return;

        Ref<EditorMenuPopup> contextMenu = BuildNoSelectionContextMenu();

        GetContext()->PushLocalPopup(contextMenu.Get(), globalMousePos, Vec2());
    }

    void AssetBrowserGridView::ShowAssetContextMenu(Vec2 globalMousePos)
    {
        Array<AssetBrowserItem*> selectedItems = GetSelectedItems();
        if (selectedItems.IsEmpty())
            return;

        //Ref<EditorMenuPopup> contextMenu = CreateObject<EditorMenuPopup>(this, "ContextMenu");
        contextMenu->QueueDestroyAllItems();

        contextMenu->MinWidth(MinContextMenuWidth);
        contextMenu->AutoClose(true);

        BuildBasicContextMenu(*contextMenu);

        GetContext()->PushLocalPopup(contextMenu.Get(), globalMousePos, Vec2());
    }

    void AssetBrowserGridView::StartRenaming()
    {
        Array<AssetBrowserItem*> selectedItems = GetSelectedItems();
        if (selectedItems.GetSize() != 1)
            return;
        if (selectedItems[0]->IsReadOnly())
            return;

        selectedItems[0]->StartEditing();
    }

    void AssetBrowserGridView::DeleteSelectedItems()
    {
        Array<AssetBrowserItem*> selectedItems = GetSelectedItems();
        if (selectedItems.IsEmpty())
            return;

        Array<CE::Name> pathsToDelete;

        for (AssetBrowserItem* selectedItem : selectedItems)
        {
            if (selectedItem->IsReadOnly())
                return;

            pathsToDelete.Add(selectedItem->GetFullPath());
        }

        if (auto owner = m_Owner.Lock())
        {
            owner->DeleteDirectoriesAndAssets(pathsToDelete);
        }
    }

    Ref<EditorMenuPopup> AssetBrowserGridView::BuildNoSelectionContextMenu()
    {
        //Ref<EditorMenuPopup> contextMenu = CreateObject<EditorMenuPopup>(this, "ContextMenu");
        contextMenu->QueueDestroyAllItems();

        contextMenu->AutoClose(true);
        contextMenu->MinWidth(MinContextMenuWidth);

        contextMenu->Content(
            FNew(FMenuItemSeparator)
            .Title("FOLDER"),

            NewMenuItem()
            .Text("New Folder")
            .Icon(FBrush("/Editor/Assets/Icons/NewFolder"))
            .OnClick([this]
            {
	            if (auto owner = m_Owner.Lock())
	            {
                    owner->CreateNewEmptyDirectory();
	            }
            }),

            FNew(FMenuItemSeparator)
            .Title("BASIC ASSETS"),

            NewMenuItem()
            .Text("Material")
            .Icon(FBrush("/Editor/Assets/Icons/Material"))
            .OnClick([this]
            {
                if (auto owner = m_Owner.Lock())
                {
                    owner->CreateNewAsset<CE::Material>();
                }
            })
        );

        return contextMenu;
    }

    bool AssetBrowserGridView::BuildBasicContextMenu(EditorMenuPopup& contextMenu)
    {
        Array<AssetBrowserItem*> selectedItems = GetSelectedItems();
        const int count = selectedItems.GetSize();
        if (count == 0)
            return false;

        bool canBeModified = true;
        bool homogenousTypes = true;
        PathTreeNodeType itemType = (PathTreeNodeType)-1;

        for (AssetBrowserItem* item : selectedItems)
        {
            auto curItemType = item->IsDirectory() ? PathTreeNodeType::Directory : PathTreeNodeType::Asset;

            if (itemType == (PathTreeNodeType)-1)
            {
                itemType = curItemType;
            }
            else if (itemType != curItemType && homogenousTypes)
            {
                homogenousTypes = false;
                itemType = (PathTreeNodeType)-1;
            }

	        if (item->IsReadOnly())
	        {
                canBeModified = false;
	        }
        }

        contextMenu
        .Content(
            FNew(FMenuItemSeparator)
            .Title("BASIC"),

            NewMenuItem()
            .Text("Copy")
            .Icon(FBrush("/Editor/Assets/Icons/Copy"))
        );

    	if (canBeModified)
        {
            if (count == 1)
            {
                contextMenu
            	.Content(
                    NewMenuItem()
                    .Text("Rename")
                    .Icon(FBrush("/Editor/Assets/Icons/Rename"))
                    .OnClick(FUNCTION_BINDING(this, StartRenaming))
                );
            }

	        contextMenu
            .Content(
                NewMenuItem()
                .Text("Delete")
                .Icon(FBrush("/Engine/Resources/Icons/Delete"))
                .OnClick(FUNCTION_BINDING(this, DeleteSelectedItems))
            );
        }

        return true;
    }

    FMenuItem& AssetBrowserGridView::NewMenuItem()
    {
        return 
        FNew(FMenuItem)
        //.Text("New Folder")
        .IconEnabled(true)
        //.Icon(FBrush("/Editor/Assets/Icons/NewFolder"))
        .FontSize(9)
        .ContentPadding(Vec4(5, -2, 5, -2));
    }

    void AssetBrowserGridView::HandleEvent(FEvent* event)
    {
        KeyModifier ctrlMod = KeyModifier::Ctrl;
        if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
        {
            ctrlMod = KeyModifier::Gui; // Gui is Cmd on mac
        }

        if (event->IsMouseEvent())
        {
            auto mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->sender == this && mouseEvent->buttons == MouseButtonMask::Left &&
                mouseEvent->type == FEventType::MousePress)
            {
                mouseEvent->Consume(this);

                DeselectAll();
            }
            else if (mouseEvent->sender == this && mouseEvent->buttons == MouseButtonMask::Right && 
                mouseEvent->type == FEventType::MousePress)
            {
                mouseEvent->Consume(this);

                if (EnumHasAnyFlags(mouseEvent->keyModifiers, KeyModifier::Shift | ctrlMod) &&
                    GetSelectedItemCount() > 0)
                {
                    // Right-click on items
                    ShowAssetContextMenu(mouseEvent->mousePosition);
                }
                else
                {
                    DeselectAll();

                    OnBackgroundRightClicked(mouseEvent->mousePosition);
                }
            }
        }

        Super::HandleEvent(event);

        if (event->IsMouseEvent())
        {
            auto mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->sender != nullptr && mouseEvent->sender->IsOfType<AssetBrowserItem>())
            {
                if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Right)
                {
                    Array<AssetBrowserItem*> selection = GetSelectedItemCount();

                    if (selection.GetSize() == 1) // Single selection
                    {
                        ShowAssetContextMenu(mouseEvent->mousePosition);
                    }
                    else if (selection.GetSize() > 1) // Multiple selection
                    {

                    }
                }
                else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left)
                {
                    // TODO: Implement shift clicking to select multiple widgets in row
                }
            }
        }
    }

    
}

