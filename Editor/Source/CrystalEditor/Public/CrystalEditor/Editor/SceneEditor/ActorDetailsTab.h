#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API ActorDetailsTab : public EditorDockWindow
    {
        CE_CLASS(ActorDetailsTab, EditorDockWindow)
    protected:

        ActorDetailsTab();

        void Construct() override;

        FUNCTION()
        void OnComponentSelectionChanged(ComponentTreeItem* item);

        FUNCTION()
        void OnAddComponentButtonClicked(FButton* button, Vec2 mousePos);

        FUNCTION()
        void AddNewComponent(ClassType* componentClass);

    public: // - Public API -

        void SetSelectedActor(Actor* actor);

    public: // - Fusion Properties -


    protected:

        FLabel* emptyLabel = nullptr;
        FCompoundWidget* detailsContainer = nullptr;
        FStyledWidget* editorContainer = nullptr;
        Ref<ObjectEditor> editor = nullptr;
        Ref<AddComponentMenu> addComponentMenu;

        FLabel* actorName = nullptr;
        ComponentTreeView* treeView = nullptr;

        FUSION_WIDGET;
    };
    
}

#include "ActorDetailsTab.rtti.h"
