#pragma once

namespace CE::Editor
{

    CLASS()
    class EDITORCORE_API ComponentTreeView : public FStyledWidget
    {
        CE_CLASS(ComponentTreeView, FStyledWidget)
    protected:

        ComponentTreeView();

        void Construct() override;

    public: // - Public API -

        void SetActor(Actor* actor);

        void Update();

        void SelectItem(ComponentTreeItem* item);

        void Expand(ComponentTreeViewRow* expandRow);
        void Collapse(ComponentTreeViewRow* collapseRow);

        void ToggleExpand(ComponentTreeViewRow* row);

        ComponentTreeItem* GetSelectedItem() const { return selectedItem; }

        Ref<ComponentTreeItem> GetActorItem() const { return actorItem; }

    public: // - Fusion Properties - 

        FUSION_EVENT(ScriptEvent<void(ComponentTreeItem*)>, OnSelectionChanged);

    protected:

        Actor* actor = nullptr;
        FVerticalStack* content = nullptr;
        FScrollBox* scrollBox = nullptr;
        Array<ComponentTreeViewRow*> rows;
        Array<ComponentTreeItem*> items;
        HashSet<ComponentTreeItem*> expandedItems;

        ComponentTreeItem* selectedItem = nullptr;
        Ref<ComponentTreeItem> actorItem;

        FUSION_WIDGET;
        friend class ComponentTreeViewStyle;
    };
    
}

#include "ComponentTreeView.rtti.h"
