#include "EditorCore.h"

namespace CE
{

    EditorDockspaceStyle::EditorDockspaceStyle()
    {
        background = Color::RGBA(26, 26, 26);

        tabWellStyle = CreateDefaultSubobject<FDockTabWellStyle>("DockTabWell");
    }

    EditorDockspaceStyle::~EditorDockspaceStyle()
    {

    }

    SubClass<FWidget> EditorDockspaceStyle::GetWidgetClass() const
    {
        return EditorDockspace::StaticType();
    }

    void EditorDockspaceStyle::MakeStyle(FWidget& widget)
    {
		Super::MakeStyle(widget);

        
    }
    
} // namespace CE

