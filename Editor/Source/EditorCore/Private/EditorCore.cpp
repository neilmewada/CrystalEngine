
#include "EditorCore.h"

namespace CE::Editor
{
	Ref<EditorStyle> gEditorStyle = nullptr;
	
    void EditorCoreModule::StartupModule()
    {
        
    }

    void EditorCoreModule::ShutdownModule()
    {

		if (ProjectManager::TryGet())
		{
            ProjectManager::TryGet()->BeginDestroy();
		}
    }

    void EditorCoreModule::RegisterTypes()
    {

    }

    void EditorCoreModule::DeregisterTypes()
    {

    }

    void EditorCoreModule::InitializeStyle(EditorStyle* style)
    {
    	gEditorStyle = style;

    	auto app = FusionApplication::TryGet();

    	if (app)
    	{
    		FStyleManager* styleManager = app->GetStyleManager();

    		FRootContext* rootContext = app->GetRootContext();

    		gEditorStyle->Initialize();

    		styleManager->RegisterStyleSet(gEditorStyle.Get());
    		rootContext->SetDefaultStyleSet(gEditorStyle.Get());
    	}
    }

    void EditorCoreModule::ShutdownStyle()
    {
    	if (gEditorStyle)
    	{
    		gEditorStyle->Shutdown();

    		gEditorStyle->BeginDestroy();
    		gEditorStyle = nullptr;
    	}
    }

    Ref<EditorStyle> EditorCoreModule::GetEditorStyle()
    {
    	return gEditorStyle;
    }

} // namespace CE

#include "EditorCore.private.h"

CE_IMPLEMENT_MODULE(EditorCore, CE::Editor::EditorCoreModule)

