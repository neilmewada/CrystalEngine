#include "EditorCore.h"

namespace CE::Editor
{

    ActorComponentTreeView::ActorComponentTreeView()
    {

    }

    void ActorComponentTreeView::Construct()
    {
        Super::Construct();

        Super::Construct();

        GenerateRowCallback(MemberDelegate(&Self::GenerateRow, this));

        RowHeight(16);
    }

    FListViewRow& ActorComponentTreeView::GenerateRow()
    {
        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        return
            FNew(FListViewRow)
            .Child(
                FNew(FHorizontalStack)
                .Gap(2.5f)
                .ContentHAlign(HAlign::Left)
                .VAlign(VAlign::Center)
                .HAlign(HAlign::Left)
                .Padding(Vec4(10, 0, 10, 0))
                (
                    FNew(FLabel)
                    .Text("Title")
                    .FontSize(fontSize)
                )
            )
            .As<FListViewRow>();
    }
}

