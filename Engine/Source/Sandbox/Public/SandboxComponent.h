#pragma once

namespace CE
{
    CLASS()
    class SANDBOX_API SandboxComponent : public ActorComponent
    {
        CE_CLASS(SandboxComponent, ActorComponent)
    public:

        virtual ~SandboxComponent();

        // - Functions -

        void SetupScene();

    protected:

        SandboxComponent();

        void Tick(f32 delta) override;

        f32 elapsedTime = 0;

        DirectionalLight* sunActor = nullptr;
        DirectionalLightComponent* sunLight = nullptr;

        CameraComponent* cameraComponent = nullptr;


    public:

        FIELD(EditAnywhere, Category = "Sandbox")
        float cameraMoveSpeed = 10.0f;

        FIELD(EditAnywhere, Category = "Sandbox")
        float cameraRotateSpeed = 100.0f;
    };
    
} // namespace CE

#include "SandboxComponent.rtti.h"
