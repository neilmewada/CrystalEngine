#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API SceneEditor : public EditorBase, public ISceneSubsystemCallbacks
    {
        CE_CLASS(SceneEditor, EditorBase)
    public:

        // - Public API -

        ClassType* GetTargetObjectType() const override;

        bool AllowMultipleInstances() const override { return false; }

        Ref<Object> GetTargetObject() const override { return targetScene; }

        bool CanEdit(Ref<Object> targetObject) const override;

        Array<CE::Name> GetSelectedAssetPaths() override;

        void BrowseToAsset(const CE::Name& path) override;

        bool OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle) override;

    protected:

        SceneEditor();

        void LoadSandboxScene();

        void LoadEmptyScene();

        void Construct() override;

        void OnBeginDestroy() override;

        void OnSceneLoaded(CE::Scene* scene) override;

        void OnAssetUnloaded(Uuid bundleUuid) override;

        FUNCTION()
        void OnActorSelectionChanged(FItemSelectionModel* selectionModel);

        // - ToolBar Controls -

        FUNCTION()
        void OnClickPlay();

        FUNCTION()
        void OnClickPause();

        FUNCTION()
        void OnClickStop();

        FUNCTION()
        void OnClickAddActorMenuButton();

        FSplitBox* rootSplitBox = nullptr;

    private:

        bool OpenScene(Ref<CE::Scene> scene);

        void ConstructMenuBar() override;
        void ConstructToolBar() override;
        void ConstructDockspaces();

        EditorMinorDockspace* rightTop = nullptr;
        EditorMinorDockspace* rightBottom = nullptr;
        EditorMinorDockspace* center = nullptr;
        EditorMinorDockspace* bottom = nullptr;

        Ref<EditorViewportTab> viewportTab = nullptr;
        Ref<SceneOutlinerTab> sceneOutlinerTab = nullptr;
        Ref<AssetBrowser> assetBrowser;
        Ref<ActorDetailsTab> detailsTab = nullptr;

        // Toolbar
        Ref<FImageButton> playButton;
        Ref<FImageButton> pauseButton;
        Ref<FImageButton> stopButton;

        Ref<FImageButton> addActorButton;

        // Context Menus
        Ref<EditorMenuPopup> addActorContextMenu;

        // Sandbox
        Ref<CE::Scene> sandboxScene = nullptr;

        Ref<CE::Scene> targetScene;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "SceneEditor.rtti.h"
