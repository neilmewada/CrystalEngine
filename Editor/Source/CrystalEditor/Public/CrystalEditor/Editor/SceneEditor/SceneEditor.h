#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API SceneEditor : public EditorBase, public ISceneSubsystemCallbacks
    {
        CE_CLASS(SceneEditor, EditorBase)
    public:

        // - Public API -

        bool CanBeClosed() const override { return false; }

    protected:

        SceneEditor();

        void LoadSandboxScene();

        void Construct() override;

        void OnBeginDestroy() override;

        void OnSceneLoaded(CE::Scene* scene) override;

        FUNCTION()
        void OnSelectionChanged(FItemSelectionModel* selectionModel);

        FSplitBox* rootSplitBox = nullptr;

    private:

        void ConstructMenuBar();
        void ConstructDockspaces();

        EditorMinorDockspace* rightTop = nullptr;
        EditorMinorDockspace* rightBottom = nullptr;
        EditorMinorDockspace* center = nullptr;
        EditorMinorDockspace* bottom = nullptr;

        EditorViewportTab* viewportTab = nullptr;

        SceneOutlinerTab* sceneOutlinerTab = nullptr;

        Ref<AssetBrowser> assetBrowser;

        DetailsTab* detailsTab = nullptr;

        // Sandbox
        CE::Scene* sandboxScene = nullptr;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "SceneEditor.rtti.h"
