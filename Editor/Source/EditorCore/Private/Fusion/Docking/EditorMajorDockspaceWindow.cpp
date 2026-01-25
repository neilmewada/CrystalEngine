#include "EditorCore.h"

namespace CE::Editor
{

    EditorMajorDockspaceWindow::EditorMajorDockspaceWindow()
    {
		m_DockspaceClass = EditorMajorDockspace::StaticClass();
    }

    void EditorMajorDockspaceWindow::Construct()
    {
        Super::Construct();
        
    }

    bool EditorMajorDockspaceWindow::OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle)
    {
        if (!targetObject)
            return false;

        Ref<AssetEditorRegistry> registry = AssetEditorRegistry::Get();
        SubClass<EditorBase> editorClass = registry->GetEditorClass(targetObject->GetClass());
        if (!editorClass.IsValid())
            return false;

        auto editorClassDefaults = (const EditorBase*)editorClass->GetDefaultInstance();
        if (!editorClassDefaults)
            return false;
        if (!editorClassDefaults->CanEdit(targetObject))
            return false;

        bool allowMultipleInstances = editorClassDefaults->AllowMultipleInstances();
        Ref<EditorBase> existingEditor = nullptr;
        int existingEditorIndex = -1;

		Ref<FDockspace> dockspace = GetDockspace();
        if (!dockspace.IsValid())
			return false;

		Ref<FDockspaceSplitView> rootSplit = dockspace->GetRootSplit();
        if (!rootSplit.IsValid())
			return false;

        int tabbedWindowCount = rootSplit->GetTabbedDockWindowCount();

        for (int i = 0; i < tabbedWindowCount; i++)
        {
            if (Ref<FDockWindow> dockWindow = rootSplit->GetTabbedDockWindow(i))
            {
	            if (Ref<EditorBase> editor = CastTo<EditorBase>(dockWindow))
	            {
		            if (editor->IsOfType(editorClass))
		            {
                        if (!allowMultipleInstances)
                        {
                            existingEditor = editor;
                            existingEditorIndex = i;
                        }

                        if (editor->GetTargetObject() == targetObject)
                        {
                            rootSplit->SetActiveTab(i);
                            return true;
                        }
		            }
	            }
            }
        }

        bool success;

        if (existingEditorIndex != -1)
        {
			rootSplit->SetActiveTab(existingEditorIndex);

            success = existingEditor->OpenEditor(targetObject, bundle);
        }
        else
        {
            Ref<EditorBase> editor = CreateObject<EditorBase>(this, editorClass->GetName().GetLastComponent(),
                OF_NoFlags, editorClass);
            existingEditor = editor;

            rootSplit->AddDockWindow(editor);
			rootSplit->SetActiveTab(rootSplit->GetTabbedDockWindowCount() - 1);

            success = editor->OpenEditor(targetObject, bundle);
        }

        if (success)
        {
            existingEditor->OnEditorOpened(targetObject);
        }

        return success;
    }

    bool EditorMajorDockspaceWindow::OpenEditor(const CE::Name& assetPath)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        if (!registry)
            return false;

        AssetData* primaryAssetData = registry->GetPrimaryAssetByPath(assetPath);
        if (!primaryAssetData)
            return false;

        Ref<Asset> asset = AssetManager::Get()->LoadAssetAtPath(assetPath);
        if (!asset)
			return false;

        return OpenEditor(asset, asset->GetBundle());
    }

    bool EditorMajorDockspaceWindow::SelectActiveEditor(Ref<EditorBase> targetEditor)
    {
        Ref<FDockspace> dockspace = GetDockspace();
        if (!dockspace.IsValid())
            return false;

        Ref<FDockspaceSplitView> rootSplit = dockspace->GetRootSplit();
        if (!rootSplit.IsValid())
            return false;

        int tabbedWindowCount = rootSplit->GetTabbedDockWindowCount();

        for (int i = 0; i < tabbedWindowCount; i++)
        {
            if (Ref<FDockWindow> dockWindow = rootSplit->GetTabbedDockWindow(i))
            {
                if (Ref<EditorBase> editor = CastTo<EditorBase>(dockWindow))
                {
                    if (editor == targetEditor)
                    {
                        rootSplit->SetActiveTab(i);
                        return true;
					}
                }
            }
        }

        return false;
    }
}

