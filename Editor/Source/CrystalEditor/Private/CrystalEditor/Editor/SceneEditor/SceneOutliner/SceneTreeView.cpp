#include "CrystalEditor.h"

namespace CE::Editor
{

    SceneTreeView::SceneTreeView()
    {

    }

    void SceneTreeView::Construct()
    {
        Super::Construct();
        
        (*this)
			.GenerateRowDelegate(MemberDelegate(&Self::GenerateRow, this))
            ;

        Style("TreeView");
    }

    FTreeViewRow& SceneTreeView::GenerateRow()
    {
        FTreeViewRow& row = FNew(FTreeViewRow);

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        row.Cells(
            FNew(FTreeViewCell)
            .Text("Name")
            .ArrowEnabled(true)
            .FontSize(fontSize),

            FNew(FTreeViewCell)
            .Text("Type")
            .FontSize(fontSize)
            .Foreground(Color::RGBA(255, 255, 255, 140))
            .ArrowEnabled(false)
        );

        return row;
    }

}

