#pragma once

namespace CE::Editor
{
    class ComponentTreeView;

    CLASS()
    class EDITORCORE_API ComponentTreeItem : public Object
    {
        CE_CLASS(ComponentTreeItem, Object)
    protected:

        ComponentTreeItem();


    public: // - Public API -

        Actor* GetActor() const { return actor; }

        ActorComponent* GetComponent() const { return component; }

        bool IsSceneComponent() const { return component != nullptr && component->IsOfType<SceneComponent>(); }

        bool IsActor() const { return actor != nullptr && component == nullptr; }

        bool IsActorComponent() const { return component != nullptr && !IsActor() && !IsSceneComponent(); }

    protected:

        Actor* actor = nullptr;
        ActorComponent* component = nullptr;
        ComponentTreeView* owner = nullptr;

        friend class ComponentTreeView;
    };
    
}

#include "ComponentTreeItem.rtti.h"
