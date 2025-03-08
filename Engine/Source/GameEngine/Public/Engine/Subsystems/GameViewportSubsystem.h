#pragma once

namespace CE
{
    CLASS()
    class GAMEENGINE_API GameViewportSubsystem : public EngineSubsystem
    {
        CE_CLASS(GameViewportSubsystem, EngineSubsystem)
    public:

        GameViewportSubsystem();
        ~GameViewportSubsystem();

        void Initialize() override;
        void PostInitialize() override;

        void PreShutdown() override;
        void Shutdown() override;

        f32 GetTickPriority() const override;

        FGameWindow* GetGameWindow() const { return gameWindow; }

        void SetScene(CE::Scene* scene);

    private:

        FGameWindow* gameWindow = nullptr;
        CE::Scene* scene = nullptr;

    };
    
} // namespace CE

#include "GameViewportSubsystem.rtti.h"