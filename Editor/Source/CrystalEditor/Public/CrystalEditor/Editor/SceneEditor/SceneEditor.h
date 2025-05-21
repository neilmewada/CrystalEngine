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

        ClassType* GetTargetObjectType() const override;

        bool AllowMultipleInstances() const override { return false; }

        Ref<Object> GetTargetObject() const override { return sandboxScene; }

        bool CanEdit(Ref<Object> targetObject) const override;

        Array<CE::Name> GetSelectedAssetPaths() override;

        void BrowseToAsset(const CE::Name& path) override;

    protected:

        SceneEditor();

        void LoadSandboxScene();

        void Construct() override;

        void OnBeginDestroy() override;

        void OnSceneLoaded(CE::Scene* scene) override;

        void OnAssetUnloaded(Uuid bundleUuid) override;

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
        Ref<CE::Scene> sandboxScene = nullptr;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "SceneEditor.rtti.h"
