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
                    .MinHeight(25)
                    .Padding(Vec4(1, 1, 1, 1) * 2.5f)
                    (
                        FNew(FWidget)
                        .FillRatio(1.0f),

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

                    FNew(FScrollBox)
                    .VerticalScroll(true)
                    .HorizontalScroll(false)
                    .OnBackgroundClicked([this]
                    {
                        gridView->DeselectAll();
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

        treeViewModel = CreateObject<AssetBrowserTreeViewModel>(this, "TreeViewModel");
        treeViewModel->Init();
        treeView->Model(treeViewModel.Get());

        gridViewModel = CreateObject<AssetBrowserGridViewModel>(this, "GridViewModel");
        gridViewModel->Init();
        gridView->Model(gridViewModel);

        currentPath = "/";
        currentDirectory = AssetRegistry::Get()->GetCachedPathTree().GetRootNode();
        UpdateAssetGridView();
    }

    void AssetBrowser::OnAssetPathTreeUpdated(PathTree& pathTree)
    {
        treeView->OnModelUpdate();
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
                currentPath = {};
                UpdateAssetGridView();
            }
            return;
        }

        for (const FModelIndex& index : selection)
        {
            if (!index.IsValid() || index.GetDataPtr() == nullptr)
                continue;

            PathTreeNode* node = (PathTreeNode*)index.GetDataPtr();

            /*if (!directoryTree.GetRootNode()->ChildExistsRecursive(node))
            {
                // Directory was deleted!
                selectionModel->ClearSelection(); // This will recursively call current function again
                break;
            }*/

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

    void AssetBrowser::UpdateAssetGridView()
    {
        gridViewModel->SetCurrentDirectory(currentPath);

        gridView->OnModelUpdate();
    }

    void AssetBrowser::SetCurrentPath(const CE::Name& path)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return;

        PathTreeNode* node = registry->GetCachedPathTree().GetNode(path);
        if (!node || node->nodeType != PathTreeNodeType::Directory)
            return;
        FModelIndex index = treeViewModel->FindIndex(node);
        if (!index.IsValid())
            return;

        treeView->SelectionModel()->Select(index);
        treeView->ExpandRow(treeViewModel->GetParent(index), true);
    }


}

