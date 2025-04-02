
#include "EditorEngine.h"

#include "EditorEngine.private.h"


namespace CE::Editor
{
	EDITORENGINE_API AssetDefinitionRegistry* gAssetDefinitionRegistry = nullptr;

    EDITORENGINE_API Ref<EditorEngine> gEditor = nullptr;

    class EditorEngineModule : public CE::Module
    {
    public:
        virtual void StartupModule() override
        {
            gEditor = CreateObject<EditorEngine>(nullptr, "EditorEngine", OF_Transient);
            gEngine = gEditor.Get();
        }

        virtual void ShutdownModule() override
        {
			gAssetDefinitionRegistry->BeginDestroy();
			gAssetDefinitionRegistry = nullptr;

			gEngine->BeginDestroy();
			gEngine = nullptr;
            gEditor = nullptr;
        }

        virtual void RegisterTypes() override
        {
			CE_REGISTER_TYPES(AssetDefinitionRegistry);

			gAssetDefinitionRegistry = CreateObject<AssetDefinitionRegistry>(nullptr, "AssetDefinitionRegistry");
        }
    };

    EDITORENGINE_API AssetDefinitionRegistry* GetAssetDefinitionRegistry()
    {
        return gAssetDefinitionRegistry;
    }
}

CE_IMPLEMENT_MODULE(EditorEngine, CE::Editor::EditorEngineModule)
