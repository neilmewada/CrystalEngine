#include "EditorCore.h"

namespace CE::Editor
{

    EditorDockspace::EditorDockspace()
    {

    }

    void EditorDockspace::Construct()
    {
        Super::Construct();

        Style("EditorDockspace");
    }

    void EditorDockspace::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        
    }

}

