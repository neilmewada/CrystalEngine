#include "CrystalEditor.h"

namespace CE
{
    // TODO: Use Actor's UUID instead of a raw pointer to the actor in FModelIndex::data
    // Raw pointer to actor can create crashes if the actor was destroyed!

    SceneTreeViewModel::SceneTreeViewModel()
    {

    }

    SceneTreeViewModel::~SceneTreeViewModel()
    {

    }

    FModelIndex SceneTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        ZoneScoped;

        if (!scene)
            return {};

        if (!parent.IsValid())
        {
            if (row >= scene->GetRootActorCount())
                return {};

            return CreateIndex(row, column, scene->GetRootActor(row));
        }

        Actor* parentActor = parent.GetData().GetValue<Actor*>();
        if (parentActor == nullptr || row >= parentActor->GetChildCount())
            return {};

        return CreateIndex(row, column, parentActor->GetChild(row));
    }

    FModelIndex SceneTreeViewModel::GetParent(const FModelIndex& index)
    {
        ZoneScoped;

        Actor* actor = index.GetData().GetValue<Actor*>();
        if (!actor)
            return {};

        if (Actor* parentActor = actor->GetParentActor())
        {
            int parentActorIndex = -1;
            if (Actor* parentsParentActor = parentActor->GetParentActor())
            {
                parentActorIndex = parentsParentActor->GetIndexOfActor(parentActor);
            }
            else
            {
                parentActorIndex = scene->GetIndexOfActor(parentActor);
            }

            if (parentActorIndex < 0)
                return {};

            return CreateIndex(parentActorIndex, 0, parentActor);
        }

        return {};
    }

    FModelIndex SceneTreeViewModel::FindIndex(Actor* actor)
    {
        ZoneScoped;

        if (!actor || !actor->GetScene())
            return {};

        if (scene != actor->GetScene())
            return {};

        if (actor->GetParentActor() == nullptr)
        {
            int index = scene->GetIndexOfActor(actor);
            if (index < 0)
                return {};

            return GetIndex(index, 0, {});
        }
        
        FModelIndex parentIndex = FindIndex(actor->GetParentActor());
        if (!parentIndex.IsValid())
            return {};

        int index = actor->GetParentActor()->GetIndexOfActor(actor);
        if (index < 0)
            return {};

        return GetIndex(index, 0, parentIndex);
    }

    u32 SceneTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        ZoneScoped;

        if (!scene)
            return {};

        if (!parent.IsValid())
        {
            return scene->GetRootActorCount();
        }

        Actor* parentActor = parent.GetData().GetValue<Actor*>();
        if (parentActor == nullptr)
            return {};

        return parentActor->GetChildCount();
    }

    u32 SceneTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return 2; // Name, Type
    }

    void SceneTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
    {
        ZoneScoped;

        if (!scene)
            return;
        FTreeViewRow* rowCast = Object::CastTo<FTreeViewRow>(&rowWidget);
        if (rowCast == nullptr)
            return;

        FTreeViewRow& treeRow = *rowCast;

        FModelIndex index = GetIndex(row, 0, parent);
        if (!index.IsValid() || !index.GetData().HasValue())
            return;

        Actor* actor = index.GetData().GetValue<Actor*>();
        String actorType = actor->GetType()->GetName().GetLastComponent();

        treeRow.GetCell(0)->Text(actor->GetName().GetString());
        treeRow.GetCell(1)->Text(actorType);
    }

    void SceneTreeViewModel::SetScene(CE::Scene* scene)
    {
        if (this->scene == scene)
            return;

        this->scene = scene;
        onModelUpdated(this);
    }

} // namespace CE

