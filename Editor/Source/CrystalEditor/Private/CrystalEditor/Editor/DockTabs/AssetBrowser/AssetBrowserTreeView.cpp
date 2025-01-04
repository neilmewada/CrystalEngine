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
        ;
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
            .FontSize(11)
        );

        return row;
    }
}

