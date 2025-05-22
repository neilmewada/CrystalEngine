#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowser::AssetBrowser()
    {

    }

    void AssetBrowser::Construct()
    {
        Super::Construct();

        Style("EditorMinorDockTab");

        (*this)
        .Title("Assets")
        .Content(
            FNew(FSplitBox)
            .SplitterBackground(Color::RGBA(26, 26, 26))
            .SplitterHoverBackground(Color::RGBA(128, 128, 128))
            .SplitterSize(5.0f)
            .SplitterDrawRatio(0.5f)
            .Direction(FSplitDirection::Horizontal)
            .ContentVAlign(VAlign::Fill)
            (
                FNew(FStyledWidget)
                .ClipChildren(true)
                .Padding(Vec4(1, 1, 1, 1) * 5)
                .FillRatio(0.2f)
                .VAlign(VAlign::Fill)
                (
                    FAssignNew(FExpandableSection, directorySection)
                    .Title("Content")
                    .ExpandableContent(
                        FAssignNew(AssetBrowserTreeView, treeView)
                        .OnSelectionChanged(FUNCTION_BINDING(this, OnDirectorySelectionChanged))
                        .Background(Color::RGBA(26, 26, 26))
                        .VAlign(VAlign::Fill)
                        .HAlign(HAlign::Fill)
                    )
                    .ContentFillRatio(1.0f)
                    .OnExpansionChanged(FUNCTION_BINDING(this, OnLeftExpansionChanged))
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill)
                    .Name("AssetBrowserTreeViewSection")
                ),

                FNew(FVerticalStack)
                .ContentHAlign(HAlign::Fill)
                .FillRatio(0.8f)
                (
                    FAssignNew(FHorizontalStack, searchBarStack)
                    .ContentVAlign(VAlign::Center)
                    .ContentHAlign(HAlign::Left)
                    .Gap(10.0f)
                    .MinHeight(25)
                    .Padding(Vec4(1, 1, 1, 1) * 2.5f)
                    (
                        FNew(FImageButton)
                        .Image(FBrush("/Editor/Assets/Icons/Refresh"))
                        .Width(18)
                        .Height(18)
                        .Padding(Vec4(1, 1, 1, 1) * 3)
                        .Style("Button"),

                        FAssignNew(FButton, importButton)
                        .OnClicked(FUNCTION_BINDING(this, OnImportButtonClicked))
                        .Child(
                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .Gap(5)
                            .HAlign(HAlign::Center)
                            (
                                FNew(FImage)
                                .Background(FBrush("/Editor/Assets/Icons/Import"))
                                .Width(14)
                                .Height(14),

                                FNew(FLabel)
                                .Text("Import")
                                .FontSize(9)
                            )
                        )
                        .Style("Button.Icon"),

                        FAssignNew(FButton, addButton)
                        .OnClicked(FUNCTION_BINDING(this, OnAddButtonClicked))
                        .Child(
                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .Gap(5)
                            .HAlign(HAlign::Center)
                            (
                                FNew(FImage)
                                .Background(FBrush("/Editor/Assets/Icons/Plus").WithTint(Color::RGBHex(0x4CAF50)))
                                .Width(14)
                                .Height(14),

                                FNew(FLabel)
                                .Text("Add")
                                .FontSize(9)
                            )
                        ),

                        FNew(FScrollBox)
                        .HideHorizontalScroll(true)
                        .HorizontalScroll(true)
                        .VerticalScroll(false)
                        .FillRatio(1.0f)
                        (
                            FAssignNew(FHorizontalStack, breadCrumbsContainer)
                            .Gap(0)
                            .VAlign(VAlign::Fill)
                            .HAlign(HAlign::Left)
                            .Padding(Vec4(1, 0, 1, 0) * 2.5f)
                        ),

                        FNew(FTextButton)
                        .Text("Settings")
                        .FontSize(9)
                        .DropDownMenu(
                            FNew(FMenuPopup)
                            .Content(
                                FNew(FMenuItem)
                                .Text("Option 1"),

                                FNew(FMenuItem)
                                .Text("Option 2")
                            )
                            .BlockInteraction(false)
                            .AutoClose(true)
                            .As<FMenuPopup>()
                        )
                    ),

                    FNew(FStyledWidget)
                    .Background(Color::RGBA(26, 26, 26))
                    .HAlign(HAlign::Fill)
                    .Height(0.5f),

                    FAssignNew(FScrollBox, gridViewScrollBox)
                    .VerticalScroll(true)
                    .HorizontalScroll(false)
                    .OnEvent([this] (FEvent* event)
                    {
                        if (event->IsMouseEvent() && event->sender == gridViewScrollBox.Get())
                        {
                            auto* mouseEvent = static_cast<FMouseEvent*>(event);

                            if (mouseEvent->type == FEventType::MousePress && mouseEvent->IsLeftButton())
                            {
                                gridView->DeselectAll();
                            }
                            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->IsRightButton())
                            {
                                if (gridView->GetSelectedItemCount() == 0 || !mouseEvent->IsMultiSelectionModifier())
                                {
                                    gridView->DeselectAll();

                                    gridView->OnBackgroundRightClicked(mouseEvent->mousePosition);
                                }
                                else
                                {
                                    gridView->ShowAssetContextMenu(mouseEvent->mousePosition);
                                }
                            }
                        }
                    })
                    .HAlign(HAlign::Fill)
                    .FillRatio(1.0f)
                    (
                        FAssignNew(AssetBrowserGridView, gridView)
                        .Owner(this)
                        .Gap(Vec2(10, 10))
                        .HAlign(HAlign::Fill)
                        .VAlign(VAlign::Top)
                        .Padding(Vec4(1, 1, 0.5f, 1) * 10.0f)
                    )
                )
            )
        );

        leftSections = { directorySection };

        AssetRegistry* registry = AssetRegistry::Get();
        registry->AddRegistryListener(this);

        treeViewModel = CreateObject<AssetBrowserTreeViewModel>(this, "TreeViewModel");
        treeViewModel->Init();
        treeView->Model(treeViewModel.Get());

        gridViewModel = CreateObject<AssetBrowserGridViewModel>(this, "GridViewModel");
        gridViewModel->Init();

        currentPath = "/";
        currentDirectory = AssetRegistry::Get()->GetCachedPathTree().GetRootNode();
        UpdateAssetGridView();

        SetCurrentPath("/Game/Assets");
    }

    void AssetBrowser::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        AssetRegistry* registry = AssetRegistry::Get();
        if (registry)
        {
            registry->RemoveRegistryListener(this);
        }
    }

    void AssetBrowser::OnAssetPathTreeUpdated(PathTree& pathTree)
    {
        treeView->OnModelUpdate();

        UpdateAssetGridView();
    }

    void AssetBrowser::OnDirectorySelectionChanged(FItemSelectionModel* selectionModel)
    {
        if (selectionModel == nullptr || treeViewModel == nullptr)
            return;

        AssetRegistry* registry = AssetManager::Get()->GetRegistry();
        if (registry == nullptr)
            return;
        PathTree& directoryTree = registry->GetCachedDirectoryPathTree();

        const HashSet<FModelIndex>& selection = selectionModel->GetSelection();

        if (selection.IsEmpty())
        {
            if (currentDirectory != nullptr)
            {
                currentDirectory = nullptr;
                currentPath = "/";
                UpdateAssetGridView();
            }
            return;
        }

        for (const FModelIndex& index : selection)
        {
            if (!index.IsValid() || !index.GetData().HasValue())
                continue;

            PathTreeNode* node = (PathTreeNode*)index.GetData().GetValue<PathTreeNode*>();

            if (currentDirectory != node)
            {
                currentDirectory = node;
                currentPath = node->GetFullPath();
                UpdateAssetGridView();
            }

            break;
        }
    }

    void AssetBrowser::OnLeftExpansionChanged(FExpandableSection* section)
    {
        for (Ref<FExpandableSection> expandableSection : leftSections)
        {
	        if (expandableSection != section)
	        {
		        expandableSection->Expanded(false);
	        }
        }
    }

    void AssetBrowser::OnAddButtonClicked()
    {
        if (!currentPath.IsValid())
            return;
        if (!currentPath.GetString().StartsWith("/Game/Assets"))
            return;

        Ref<EditorMenuPopup> contextMenu = gridView->BuildNoSelectionContextMenu();

        Vec2 popupPos = addButton->GetGlobalPosition() + Vec2(0, addButton->GetComputedSize().y);

        GetContext()->PushLocalPopup(contextMenu.Get(), popupPos, Vec2(), addButton->GetComputedSize());
    }

    void AssetBrowser::OnImportButtonClicked()
    {
        if (!currentPath.IsValid())
            return;
        if (!currentPath.GetString().StartsWith("/Game/Assets"))
            return;

        Array<EditorPlatformBase::FileType> fileTypes;

        AssetDefinitionRegistry* registry = GetAssetDefinitionRegistry();
        for (int i = 0; i < registry->GetAssetDefinitionsCount(); ++i)
        {
            AssetDefinition* assetDef = registry->GetAssetDefinition(i);
            fileTypes.Add({
                .desc = assetDef->GetTypeDisplayName(),
                .extensions = assetDef->GetSourceExtensions()
            });
        }

        IO::Path selectedFile = EditorPlatform::ShowFileSelectionDialog(defaultAssetImportPath.GetString(), fileTypes);

        if (selectedFile.Exists())
        {
            defaultAssetImportPath = selectedFile.GetParentPath().GetString();
            CE_LOG(Info, All, "Selected file: {}", selectedFile.GetString());
        }
    }

    void AssetBrowser::UpdateBreadCrumbs()
    {
        breadCrumbsContainer->QueueDestroyAllChildren();

        if (currentPath == "/")
        {
            breadCrumbsContainer->AddChild(
                FNew(FTextButton)
                .Text("/")
                .Interactable(false)
                .Style("Button.Icon")
                .Padding(Vec4(1, 1, 1, 1) * 5.0f)
            );

            return;
        }

        Array<String> split;
        currentPath.GetString().Split("/", split);
        String pathIterator = "";

        for (int i = 0; i < split.GetSize(); ++i)
        {
            pathIterator += "/";

            breadCrumbsContainer->AddChild(
                FNew(FTextButton)
                .Text("/")
                .Interactable(false)
                .Style("Button.Icon")
                .Padding(Vec4(1, 1, 1, 1) * 5.0f)
            );

            pathIterator += split[i];

            breadCrumbsContainer->AddChild(
                FNew(FTextButton)
                .Text(split[i])
                .OnClicked([pathIterator, this]
                {
                    SetCurrentPath(pathIterator);
                })
                .Style("Button.Icon")
                .Padding(Vec4(1, 1, 1, 1) * 5.0f)
            );
        }
    }

    void AssetBrowser::UpdateAssetGridView()
    {
        gridView->SetCurrentDirectory(currentPath);

        gridView->OnUpdate();

        UpdateBreadCrumbs();
    }

    bool AssetBrowser::IsCurrentDirectoryReadOnly() const
    {
        if (!currentPath.IsValid() || currentPath == "/")
        {
	        return true;
        }

        return !currentPath.GetString().StartsWith("/Game/Assets");
    }

    Array<CE::Name> AssetBrowser::GetSelectedAssetPaths()
    {
        Array<CE::Name> selectedAssetPaths;

        Array<AssetBrowserItem*> selectedItems = gridView->GetSelectedItems();
        for (AssetBrowserItem* selectedItem : selectedItems)
        {
            selectedAssetPaths.Add(selectedItem->GetFullPath());
        }

        return selectedAssetPaths;
    }

    void AssetBrowser::BrowseToAsset(const CE::Name& path)
    {
        CE::Name parentPath = IO::Path(path.GetString()).GetParentPath().GetString().Replace({ '\\' }, '/');

        if (currentPath == parentPath)
        {
            gridView->SelectItem(path);
        }
        else
        {
            gridView->itemToSelect = path;

            SetCurrentPath(parentPath);
        }
    }

    void AssetBrowser::CreateNewEmptyDirectory()
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return;

        EditorAssetManager* assetManager = EditorAssetManager::Get();

        PathTreeNode* curNode = registry->GetCachedPathTree().GetNode(currentPath);
        if (curNode == nullptr)
            return;

        String path = curNode->GetFullPath();

        if (!path.StartsWith("/Game/Assets"))
			return;

        IO::Path absolutePath = gProjectPath / path.GetSubstring(1);
        if (!absolutePath.Exists())
            return;

        IO::Path newFolder = absolutePath / "NewFolder";
        if (!newFolder.Exists())
        {
            gridView->folderToRename = path + "/NewFolder";
            IO::Path::CreateDirectories(newFolder);
            assetManager->OnDirectoryCreated(newFolder);
	        return;
        }

        for (int i = 1; i <= 100; i++)
        {
            newFolder = absolutePath / String::Format("NewFolder_{}", i);

            if (!newFolder.Exists())
            {
                gridView->folderToRename = path + String::Format("/NewFolder_{}", i);
                IO::Path::CreateDirectories(newFolder);
                assetManager->OnDirectoryCreated(newFolder);
	            return;
            }
        }
    }

    bool AssetBrowser::CreateNewAsset(SubClass<Asset> assetType)
    {
        if (assetType == nullptr)
            return false;

        if (assetType == CE::Scene::StaticClass())
            return false;

        // Most other asset types
        String assetTypeName = assetType->GetName().GetLastComponent();
        String baseName = "New" + assetTypeName;
        IO::Path absolutePath = gProjectPath / currentPath.GetString().GetSubstring(1) / (baseName + ".casset");
        int i = 0;

        while (absolutePath.Exists())
        {
            baseName = "New" + assetTypeName + "_" + i;
            absolutePath = gProjectPath / currentPath.GetString().GetSubstring(1) / (baseName + ".casset");
            i++;
        }

        CE::Name outputPath = currentPath.GetString() + "/" + baseName;

        Ref<Bundle> bundle = CreateObject<Bundle>(this, baseName);

        Ref<Asset> assetInstance = CreateObject<Asset>(bundle.Get(), assetTypeName, OF_NoFlags, assetType);
        assetInstance->ResetAsset();

        auto result = Bundle::SaveToDisk(bundle, nullptr, outputPath);

        bundle->BeginDestroy();
        bundle = nullptr;
        assetInstance = nullptr;

        EditorAssetManager* assetManager = EditorAssetManager::Get();
        if (assetManager)
        {
            assetManager->OnAssetCreated(Bundle::GetAbsoluteBundlePath(outputPath));
        }

        return result == BundleSaveResult::Success;
    }

    bool AssetBrowser::CanRenameDirectory(const CE::Name& originalPath, const CE::Name& newName)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return false;

        EditorAssetManager* assetManager = EditorAssetManager::Get();
        if (!assetManager)
            return false;

        String projectPath = gProjectPath.GetString();

        IO::Path absolutePath = gProjectPath / originalPath.GetString().GetSubstring(1);
        IO::Path newPath = absolutePath.GetParentPath() / newName.GetString();
        if (newPath.Exists())
            return false;

        return true;
    }

    bool AssetBrowser::RenameDirectory(const CE::Name& originalPath, const CE::Name& newName)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return false;

        EditorAssetManager* assetManager = EditorAssetManager::Get();
        if (!assetManager)
            return false;

        IO::Path absolutePath = gProjectPath / originalPath.GetString().GetSubstring(1);
        IO::Path newPath = absolutePath.GetParentPath() / newName.GetString();
        if (newPath.Exists())
            return false;

        IO::Path::Rename(absolutePath, newPath);

        assetManager->OnDirectoryRenamed(originalPath, newName);

        return true;
    }

    bool AssetBrowser::RenameAsset(const CE::Name& originalPath, const CE::Name& newName)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return false;

        EditorAssetManager* assetManager = EditorAssetManager::Get();
        if (!assetManager)
            return false;

        IO::Path absolutePath = gProjectPath / (originalPath.GetString().GetSubstring(1) + ".casset");
        IO::Path newPath = absolutePath.GetParentPath() / (newName.GetString() + ".casset");
        if (newPath.Exists() || !absolutePath.Exists())
            return false;

        IO::Path::Rename(absolutePath, newPath);

        assetManager->OnAssetRenamed(originalPath, newPath, newName);

        return true;
    }

    void AssetBrowser::DeleteDirectoriesAndAssets(const Array<CE::Name>& itemPaths)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return;

        EditorAssetManager* assetManager = EditorAssetManager::Get();
        if (!assetManager)
            return;

        if (itemPaths.IsEmpty())
            return;

        Array<IO::Path> absolutePaths;

        for (const auto& itemPath : itemPaths)
        {
            IO::Path absolutePath = gProjectPath / itemPath.GetString().GetSubstring(1);
            IO::Path absoluteBundlePath = gProjectPath / (itemPath.GetString().GetSubstring(1) + ".casset");

            if (absolutePath.Exists())
            {
                absolutePaths.Add(absolutePath);
            }
            else if (absoluteBundlePath.Exists())
            {
                absolutePaths.Add(absoluteBundlePath);
            }
        }

        for (const auto& path : absolutePaths)
        {
            if (path.Exists())
            {
                IO::Path::RemoveRecursively(path);
            }
        }

        assetManager->OnDirectoryAndAssetsDeleted(itemPaths);
    }

    void AssetBrowser::SetCurrentPath(const CE::Name& path)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return;

        PathTreeNode* node = registry->GetCachedPathTree().GetNode(path);
        if (!node || node->nodeType != PathTreeNodeType::Directory)
            return;
        if (node == registry->GetCachedPathTree().GetRootNode())
        {
            treeView->SelectionModel()->ClearSelection();
            return;
        }

        FModelIndex index = treeViewModel->FindIndex(node);
        if (!index.IsValid())
            return;

        treeView->SelectionModel()->Select(index);
        treeView->ExpandRow(treeViewModel->GetParent(index), true);
    }

    void AssetBrowser::OpenAsset(const CE::Name& path)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return;

        PathTreeNode* node = registry->GetCachedPathTree().GetNode(path);
        if (!node || node->nodeType != PathTreeNodeType::Asset)
            return;

        CE::Name fullPath = node->GetFullPath();

        CrystalEditorWindow::Get()->OpenEditor(fullPath);
    }


}

