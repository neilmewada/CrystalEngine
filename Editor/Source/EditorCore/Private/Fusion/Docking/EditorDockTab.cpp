#include "EditorCore.h"

namespace CE::Editor
{

	EditorDockTab::EditorDockTab()
    {

    }

    void EditorDockTab::Construct()
    {
        Super::Construct();

        Child(
            FNew(FVerticalStack)
            .VAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            (
                FAssignNew(EditorMenuBar, menuBar)
                .HAlign(HAlign::Fill),

                FAssignNew(EditorToolBar, toolBar)
                .HAlign(HAlign::Fill),

                FAssignNew(FStyledWidget, content)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                .FillRatio(1.0f)
                .Name("EditorContent")
            )
        );
    }

    void EditorDockTab::UpdateDockspaceTabWell()
    {
	    if (dockspace && dockspace->IsOfType<EditorDockspace>())
	    {
	        ((EditorDockspace*)dockspace)->UpdateTabs();
	    }
    }

    void EditorDockTab::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (dockspace)
        {
	        if (dockspace->IsOfType<EditorDockspace>())
	        {
                auto editorDockspace = static_cast<EditorDockspace*>(dockspace);

                editorDockspace->RemoveDockTab(this);
	        }
            else if (dockspace->IsOfType<EditorMinorDockspace>())
            {
                auto editorMinorDockspace = static_cast<EditorMinorDockspace*>(dockspace);

                editorMinorDockspace->RemoveDockTab(this);
            }
        }
    }

    EditorDockTab& EditorDockTab::Content(FWidget& widget)
    {
        content->Child(widget);
        return *this;
    }

    EditorMinorDockTab::EditorMinorDockTab()
    {
        m_TabRole = EditorDockTabRole::Minor;
    }

    void EditorMinorDockTab::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        static const CE::Name tabRole = "TabRole";

        if (propertyName == tabRole)
        {
            m_TabRole = EditorDockTabRole::Minor;
        }
    }

    void EditorMinorDockTab::Construct()
    {
	    Super::Construct();

        MenuBarEnabled(false);
        ToolBarEnabled(false);
    }

}

