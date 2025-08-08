#include "CrystalEditor.h"

namespace CE::Editor
{

    DetailsTab::DetailsTab()
    {

    }

    void DetailsTab::Construct()
    {
	    Super::Construct();

        ConstructMinorDockWindow();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        (*this)
        .Title("Details")
        .Child(
            FNew(FScrollBox)
            .VerticalScroll(true)
            .HorizontalScroll(false)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FAssignNew(FStyledWidget, editorContainer)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Top)
            )
        );
    }

    void DetailsTab::SetTargetObject(Ref<Object> object)
    {
        if (object == targetObject)
            return;

        if (editor)
        {
            editorContainer->RemoveChild(editor.Get());
            editor->BeginDestroy();
            editor = nullptr;
        }

        targetObject = object;
        if (!targetObject)
            return;

        editor = ObjectEditorRegistry::Get().Create(targetObject.Get(), GetOwnerEditor()->GetHistory());
        if (!editor)
        {
            targetObject = nullptr;
	        return;
        }

        editorContainer->AddChild(editor.Get());
    }
} // namespace CE

