#pragma once

namespace CE
{
    class Scene;

    struct ISceneSubsystemCallbacks
    {
        virtual ~ISceneSubsystemCallbacks() = default;

        virtual void OnSceneLoaded(CE::Scene* scene) {}

    };

    struct ISceneCallbacks
    {
        virtual ~ISceneCallbacks() = default;

        virtual void OnSceneHierarchyUpdated(CE::Scene* scene) {}

    };
    
} // namespace CE
