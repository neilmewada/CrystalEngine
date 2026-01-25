#include "EditorCore.h"

namespace CE::Editor
{

    ComponentTreeView::ComponentTreeView()
    {

    }

    void ComponentTreeView::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FScrollBox, scrollBox)
            .VerticalScroll(true)
            .HorizontalScroll(false)
            .Name("ComponentTreeScrollBox")
            (
                FAssignNew(FVerticalStack, content)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Auto)
            )
        );
    }

    void ComponentTreeView::SetActor(Actor* actor)
    {
        //content->QueueDestroyAllChildren();
        content->DestroyAllChildren();

        for (ComponentTreeItem* item : items)
        {
            item->BeginDestroy();
        }
        items.Clear();
        expandedItems.Clear();
        selectedItem = nullptr;
        rows.Clear();

        this->actor = actor;
        actorItem = CreateObject<ComponentTreeItem>(this, actor->GetName().GetString());
        actorItem->actor = actor;

        ComponentTreeViewRow* actorRow = nullptr;
        FAssignNew(ComponentTreeViewRow, actorRow)
            .Title(String::Format("{} (Self)", actor->GetName()))
            .IconVisible(false)
            ;

        actorRow->owner = this;
        actorRow->item = actorItem.Get();
        actorRow->parentRow = nullptr;

        content->AddChild(actorRow);
        rows.Add(actorRow);
        items.Add(actorItem.Get());
        
        SceneComponent* sceneComponent = actor->GetRootComponent();

        std::function<void(SceneComponent* component, int indent, ComponentTreeViewRow* parent)> visitor = 
            [&](SceneComponent* component, int indent, ComponentTreeViewRow* parent)
            {
                ComponentTreeViewRow* row = nullptr;
                bool hasChildren = component->GetAttachedComponentCount() > 0;

                FAssignNew(ComponentTreeViewRow, row)
				.Title(String::Format("{} ({})", component->GetName(), component->GetClass()->GetName().GetLastComponent()))
				.Indentation(indent * 20)
				.ArrowVisible(hasChildren)
				.ArrowExpanded(hasChildren)
				.IconVisible(false)
                ;

                ComponentTreeItem* item = CreateObject<ComponentTreeItem>(this, component->GetName().GetString());
                item->component = component;

                row->owner = this;
                row->item = item;
                row->parentRow = parent;

                rows.Add(row);
                content->AddChild(row);

                if (hasChildren)
                {
                    expandedItems.Add(item);
                }

                items.Add(item);

                for (int i = 0; i < component->GetAttachedComponentCount(); ++i)
                {
                    visitor(component->GetAttachedComponent(i), indent + 1, row);
                }
            };

        if (sceneComponent != nullptr)
        {
            visitor(sceneComponent, 0, nullptr);
        }

        for (int i = 0; i < actor->GetComponentCount(); ++i)
        {
            ActorComponent* component = actor->GetComponent(i);

            ComponentTreeViewRow* row = nullptr;

            FAssignNew(ComponentTreeViewRow, row)
            .Title(String::Format("{} ({})", component->GetName(), component->GetClass()->GetName().GetLastComponent()))
            .IconVisible(false)
            ;

            ComponentTreeItem* item = CreateObject<ComponentTreeItem>(this, component->GetName().GetString());
            item->component = component;

            row->owner = this;
            row->item = item;
            row->parentRow = nullptr;

            rows.Add(row);
            content->AddChild(row);

            items.Add(item);
        }

        scrollBox->NormalizedScrollY(0);

        SelectItem(actorItem.Get());
    }

    void ComponentTreeView::Update()
    {
        SetActor(actor);
    }

    void ComponentTreeView::SelectItem(ComponentTreeItem* item)
    {
        if (selectedItem == item)
            return;

        selectedItem = item;

        for (ComponentTreeViewRow* row : rows)
        {
            row->isSelected = (row->item == selectedItem);
        }

        ApplyStyle();

        m_OnSelectionChanged(item);
    }

    void ComponentTreeView::Expand(ComponentTreeViewRow* expandRow)
    {
        expandedItems.Add(expandRow->item);

        for (ComponentTreeViewRow* row : rows)
        {
            row->ArrowExpanded(expandedItems.Exists(row->item));

            if (row->parentRow == nullptr)
                continue;

            row->Enabled(expandedItems.Exists(row->parentRow->item));
        }
    }

    void ComponentTreeView::Collapse(ComponentTreeViewRow* collapseRow)
    {
        expandedItems.Remove(collapseRow->item);

        for (ComponentTreeViewRow* row : rows)
        {
            row->ArrowExpanded(expandedItems.Exists(row->item));

            if (row->parentRow == nullptr)
                continue;

            row->Enabled(expandedItems.Exists(row->parentRow->item));
        }
    }

    void ComponentTreeView::ToggleExpand(ComponentTreeViewRow* row)
    {
        if (expandedItems.Exists(row->item))
        {
            Collapse(row);
        }
        else
        {
            Expand(row);
        }
    }
}

