#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API DetailsTab : public EditorMinorDockWindow
    {
        CE_CLASS(DetailsTab, EditorMinorDockWindow)
    protected:

        DetailsTab();

        void Construct() override;

        FUNCTION()
        void OnComponentSelectionChanged(ComponentTreeItem* item);

    public: // - Public API -

        void SetSelectedActor(Actor* actor);

    public: // - Fusion Properties -


    protected:

        FLabel* emptyLabel = nullptr;
        FCompoundWidget* detailsContainer = nullptr;
        FStyledWidget* editorContainer = nullptr;
        Ref<ObjectEditor> editor = nullptr;

        FLabel* actorName = nullptr;
        ComponentTreeView* treeView = nullptr;

        FUSION_WIDGET;
    };
    
}

#include "DetailsTab.rtti.h"
