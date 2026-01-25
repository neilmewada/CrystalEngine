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
        .AutoHeight(true)
            ;

        Style("TreeView");
    }

    FTreeViewRow& SceneTreeView::GenerateRow()
    {
        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        return
    	FNew(FTreeViewRow)
    	.Cells(
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
    }

}

