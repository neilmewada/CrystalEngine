#include "EditorCore.h"

namespace CE::Editor
{

    EditorMajorDockWindow::EditorMajorDockWindow()
    {

    }

    void EditorMajorDockWindow::Construct()
    {
        Super::Construct();

        Child(
            FNew(FVerticalStack)
            .ContentHAlign(HAlign::Fill)
            .ContentVAlign(VAlign::Top)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FAssignNew(EditorMenuBar, menuBar)
                .HAlign(HAlign::Fill),

                FAssignNew(EditorToolBar, toolBar)
                .HAlign(HAlign::Fill),

                FNew(FStyledWidget)
                .Background(Color::RGBA(26, 26, 26))
                .HAlign(HAlign::Fill)
                .Height(1.0f),

                FAssignNew(EditorMinorDockspace, minorDockspace)
                .HAlign(HAlign::Fill)
                .FillRatio(1.0f)
            )
        );

        Style("EditorMajorDockWindow");
    }
    
}

