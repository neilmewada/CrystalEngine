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
        .GenerateRowDelegate(MemberDelegate(&Self::GenerateRow, this))
        .Style("TreeView")
        ;
    }

    void AssetBrowserTreeView::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

        
    }

    FTreeViewRow& AssetBrowserTreeView::GenerateRow()
    {
        FTreeViewRow& row = FNew(FTreeViewRow);

        row.Cells(
            FNew(FTreeViewCell)
            .Text("Name")
            .ArrowEnabled(true)
            .IconEnabled(true)
            .IconBackground(FBrush("/Editor/Assets/Icons/Folder"))
            .IconWidth(14)
            .IconHeight(14)
            .ContentPadding(Vec4(1.5f, 2, 1.5f, 2) * 2)
            .FontSize(10)
            .Padding(Vec4())
        );

        return row;
    }
}

