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
        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

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
                .Text("Title")
                .FontSize(fontSize),

                FNew(FLabel)
                .FontSize(fontSize - 2)
                .Text("Path")
                .Foreground(Color::RGBA(255, 255, 255, 140))
            )
        )
        .As<FListViewRow>();
    }
}

