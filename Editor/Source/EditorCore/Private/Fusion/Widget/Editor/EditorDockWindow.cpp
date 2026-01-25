#include "EditorCore.h"

namespace CE::Editor
{

    EditorDockWindow::EditorDockWindow()
    {
        m_HAlign = HAlign::Fill;
    	m_VAlign = VAlign::Fill;
    }

    void EditorDockWindow::Construct()
    {
        Super::Construct();

        
    }

    void EditorDockWindow::SetOwnerEditor(Ref<EditorBase> editor)
    {
        this->ownerEditor = editor;
    }

    void EditorDockWindow::ConstructMajorDockWindow()
    {
        AllowedDockspaces(FDockspaceFilter().WithDockTypeMask(FDockTypeMask::Major));

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

    void EditorDockWindow::ConstructMinorDockWindow()
    {
        AllowedDockspaces(FDockspaceFilter().WithDockTypeMask(FDockTypeMask::All));

        Style("EditorMinorDockWindow");
    }

}

