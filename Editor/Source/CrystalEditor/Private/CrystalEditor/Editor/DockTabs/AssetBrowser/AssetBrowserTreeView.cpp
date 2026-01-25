#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserTreeView::AssetBrowserTreeView()
    {

    }

    void AssetBrowserTreeView::Construct()
    {
        Super::Construct();

        (*this)
        .AutoHeight(true)
        .GenerateRowDelegate(MemberDelegate(&Self::GenerateRow, this))
        .Style("TreeView")
        ;

        container->Name("DebugContainer");
    }

    void AssetBrowserTreeView::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

        
    }

    FTreeViewRow& AssetBrowserTreeView::GenerateRow()
    {
        FTreeViewRow& row = FNew(FTreeViewRow);

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        row.Cells(
            FNew(FTreeViewCell)
            .Text("Name")
            .ArrowEnabled(true)
            .IconEnabled(true)
            .IconBackground(FBrush("/Editor/Assets/Icons/Folder"))
            .IconWidth(14)
            .IconHeight(14)
            .ContentPadding(Vec4(1.5f, 1.5f, 1.5f, 1.5f) * 2)
            .FontSize(fontSize)
            .Padding(Vec4())
        );

        return row;
    }
}

