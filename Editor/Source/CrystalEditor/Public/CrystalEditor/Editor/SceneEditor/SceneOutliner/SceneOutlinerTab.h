#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API SceneOutlinerTab : public EditorDockWindow, ISceneCallbacks
    {
        CE_CLASS(SceneOutlinerTab, EditorDockWindow)
    protected:

        SceneOutlinerTab();

        void Construct() override;

        void OnBeginDestroy() override;

        void OnSceneHierarchyUpdated(CE::Scene* scene) override;

        SceneTreeView* treeView = nullptr;
        SceneTreeViewModel* treeViewModel = nullptr;

    public: // - Public API -

        void SetScene(CE::Scene* scene);

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class SceneEditor;
    };
    
}

#include "SceneOutlinerTab.rtti.h"
