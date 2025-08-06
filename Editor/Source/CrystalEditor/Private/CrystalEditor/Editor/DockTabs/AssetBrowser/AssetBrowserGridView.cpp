#include "CrystalEditor.h"

namespace CE::Editor
{
    static constexpr f32 MinContextMenuWidth = 200;
    static constexpr Vec4 ContextMenuPadding = Vec4(2, 10, 2, 10);

    AssetBrowserGridView::AssetBrowserGridView()
    {
        contextMenu = CreateDefaultSubobject<EditorMenuPopup>("ContextMenu");
    }

    void AssetBrowserGridView::Construct()
    {
        Super::Construct();

        contextMenu->ContentPadding(ContextMenuPadding);
    }

    void AssetBrowserGridView::DeselectAll()
    {
        for (AssetBrowserItem* item : items)
        {
            item->Deselect();
        }

        if (Ref<AssetBrowser> assetBrowser = m_Owner.Lock())
        {
            assetBrowser->OnItemSelectionUpdated();
        }
    }

    void AssetBrowserGridView::SelectAll()
    {
        for (AssetBrowserItem* item : items)
        {
            item->Select();
        }

        if (Ref<AssetBrowser> assetBrowser = m_Owner.Lock())
        {
            assetBrowser->OnItemSelectionUpdated();
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
        CE::Name lastComponent = fullPath.GetLastComponent();

        for (AssetBrowserItem* item : items)
        {
            if (item->GetItemName() == lastComponent)
            {
                item->Select();
            }
            else
            {
                item->Deselect();
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
                    .OnSelect([this](FSelectableButton* button)
                    {
	                    if (Ref<AssetBrowser> assetBrowser = m_Owner.Lock())
                        {
                            assetBrowser->OnItemSelectionUpdated();
						}
                    })
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

        contextMenu->ClosePopup();

        BuildNoSelectionContextMenu();

        GetContext()->PushLocalPopup(contextMenu.Get(), globalMousePos, Vec2());
    }

    void AssetBrowserGridView::ShowAssetContextMenu(Vec2 globalMousePos)
    {
        Array<AssetBrowserItem*> selectedItems = GetSelectedItems();
        if (selectedItems.IsEmpty())
            return;

        contextMenu->ClosePopup();

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

            NewMenuItem()
            .Text("Import Asset(s)")
            .Icon(FBrush("/Editor/Assets/Icons/Import"))
            .OnClick([this]
            {
                if (auto owner = m_Owner.Lock())
                {
                    owner->OnImportButtonClicked();
                }
            }),

            FNew(FMenuItemSeparator)
            .Title("BASIC"),

            NewMenuItem()
            .Text("Scene")
            .Icon(FBrush("/Editor/Assets/Icons/Scene"))
            .OnClick([this]
            {
                if (auto owner = m_Owner.Lock())
                {
                    owner->CreateNewAsset<CE::Scene>();
                }
            }),

            NewMenuItem()
            .Text("Material")
            .Icon(FBrush("/Editor/Assets/Icons/Material"))
            .OnClick([this]
            {
                if (auto owner = m_Owner.Lock())
                {
                    owner->CreateNewAsset<CE::Material>();
                }
            }),

            FNew(FMenuItemSeparator)
            .Title("ADVANCED"),

            NewMenuItem()
            .Text("Physics")
            .SubMenu(
                FNew(EditorMenuPopup)
                .Content(
                    NewMenuItem()
                    .Text("Physics Material")
                    .Icon(FBrush("/Editor/Assets/Icons/Bounce"))
                    .OnClick([this]
                    {
                        if (auto owner = m_Owner.Lock())
                        {
                            owner->CreateNewAsset<CE::PhysicsMaterial>();
                        }
                    })
                )
                .ContentPadding(ContextMenuPadding)
                .MinWidth(MinContextMenuWidth)
                .As<EditorMenuPopup>()
            ),

            FNew(FMenuItemSeparator)
            .Title("MISC"),

			NewMenuItem()
            .Text("Open in " + PlatformMisc::GetSystemFileExplorerDisplayName())
            .Icon(FBrush("/Editor/Assets/Icons/OpenFolder"))
            .OnClick([this]
            {
                EditorPlatform::OpenPathInFileExplorer(Bundle::GetAbsoluteDirectoryPath(currentPath));
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
        CE::Name fullPath = selectedItems[0]->GetFullPath();
		IO::Path absolutePath = Bundle::GetAbsoluteBundlePath(fullPath);

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

        contextMenu
        .Content(
            FNew(FMenuItemSeparator)
            .Title("MISC"),

            NewMenuItem()
            .Text("Open in " + PlatformMisc::GetSystemFileExplorerDisplayName())
            .Icon(FBrush("/Editor/Assets/Icons/OpenFolder"))
            .OnClick([this, absolutePath]
            {
                EditorPlatform::OpenPathInFileExplorer(absolutePath);
            })
        );

        return true;
    }

    FMenuItem& AssetBrowserGridView::NewMenuItem()
    {
        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        return 
        FNew(FMenuItem)
        //.Text("New Folder")
        .IconEnabled(true)
        //.Icon(FBrush("/Editor/Assets/Icons/NewFolder"))
        .FontSize(fontSize)
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
        else if (event->IsKeyEvent())
        {
            FKeyEvent* keyEvent = static_cast<FKeyEvent*>(event);

            KeyModifier ctrl = KeyModifier::Ctrl;
#if PLATFORM_MAC
            ctrl = KeyModifier::Gui;
#endif

            if (keyEvent->type == FEventType::KeyPress && keyEvent->key == KeyCode::A && 
                EnumHasFlag(keyEvent->modifiers, ctrl))
            {
                SelectAll();
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

