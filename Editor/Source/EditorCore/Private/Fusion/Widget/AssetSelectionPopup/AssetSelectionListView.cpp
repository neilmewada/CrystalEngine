#include "EditorCore.h"

namespace CE::Editor
{

    AssetSelectionListView::AssetSelectionListView()
    {

    }

    void AssetSelectionListView::Construct()
    {
        Super::Construct();

        GenerateRowCallback(MemberDelegate(&Self::GenerateRow, this));

        RowHeight(46);
    }

    FListViewRow& AssetSelectionListView::GenerateRow()
    {
        return
        FNew(FListViewRow)
        .Child(
            FNew(FVerticalStack)
            .Gap(2.5f)
            .ContentHAlign(HAlign::Left)
            .VAlign(VAlign::Center)
            .HAlign(HAlign::Left)
            .Padding(Vec4(10, 0, 10, 0))
            (
                FNew(FLabel)
                .Text("Title"),

                FNew(FLabel)
                .FontSize(8)
                .Text("Path")
                .Foreground(Color::RGBA(255, 255, 255, 140))
            )
        )
        .As<FListViewRow>();
    }
}

