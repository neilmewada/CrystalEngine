#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowser::AssetBrowser()
    {

    }

    void AssetBrowser::Construct()
    {
        Super::Construct();

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
                    FNew(FExpandableSection)
                    .Title("Content")
                    .ExpandableContent(
                        FAssignNew(AssetBrowserTreeView, treeView)
                        .OnSelectionChanged(FUNCTION_BINDING(this, OnDirectorySelectionChanged))
                        .Background(Color::RGBA(26, 26, 26))
                        .VAlign(VAlign::Fill)
                        .HAlign(HAlign::Fill)
                    )
                    .ContentFillRatio(1.0f)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill)
                    .Name("AssetBrowserTreeViewSection")
                ),
                
                FNew(FVerticalStack)
                .ContentHAlign(HAlign::Fill)
                .FillRatio(0.8f)
            )
        )
        .Style("EditorMinorDockTab");

        model = CreateObject<AssetBrowserTreeViewModel>(this, "Model");

        treeView->Model(model.Get());
    }

    void AssetBrowser::OnDirectorySelectionChanged(FItemSelectionModel* selectionModel)
    {
        if (selectionModel == nullptr || model == nullptr)
            return;

        const HashSet<FModelIndex>& selection = selectionModel->GetSelection();

        for (FModelIndex index : selection)
        {
            if (!index.IsValid() || index.GetDataPtr() == nullptr)
                continue;

            PathTreeNode* node = (PathTreeNode*)index.GetDataPtr();
            CE_LOG(Info, All, "Selected: {}", node->name);

            break;
        }
    }
}

