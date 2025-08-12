#include "EditorCore.h"

namespace CE::Editor
{

    AddComponentTreeView::AddComponentTreeView()
    {

    }

    void AddComponentTreeView::Construct()
    {
        Super::Construct();

        GenerateRowDelegate(MemberDelegate(&Self::GenerateRow, this));

        RowHeight(16);

        Style("TreeView");
    }

    FTreeViewRow& AddComponentTreeView::GenerateRow()
    {
        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        return
            FNew(FTreeViewRow)
            .Cells(
                FNew(FTreeViewCell)
                .Text("Title")
                .FontSize(fontSize)
            )
            .As<FTreeViewRow>();
    }
}

