#include "CrystalEditor.h"

namespace CE::Editor
{

    EditorViewportTab::EditorViewportTab()
    {

    }

    void EditorViewportTab::Construct()
    {
        Super::Construct();

        ConstructMinorDockWindow();

        (*this)
			.Title("Viewport")
            .Child(
                FAssignNew(EditorViewport, viewport)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
            );
    }
    
}

