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

        AssetRegistry* registry = AssetRegistry::Get();
        registry->AddRegistryListener(this);

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
            if (!index.IsValid() || index.GetDataPtr() == nullptr)
                continue;

            PathTreeNode* node = (PathTreeNode*)index.GetDataPtr();

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

    void AssetBrowser::UpdateBreadCrumbs()
    {
        breadCrumbsContainer->QueueDestroyAllChildren();

        if (currentPath == "/")
        {
            breadCrumbsContainer->AddChild(
                FNew(FTextButton)
                .Text("/")
                .OnClicked([this]
                {
                    SetCurrentPath("/");
                })
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
                .OnClicked([pathIterator, this]
                {
                    SetCurrentPath(pathIterator);
                })
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
        gridViewModel->SetCurrentDirectory(currentPath);

        gridView->OnModelUpdate();

        UpdateBreadCrumbs();
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


}

