#include "Engine.h"

namespace CE
{

    DirectionalLightComponent::DirectionalLightComponent()
    {
	    if (!IsDefaultInstance())
	    {
            rpiView = RPI::View::CreateView("DirectionalLightShadow", View::UsageShadow);
	    }
    }

    DirectionalLightComponent::~DirectionalLightComponent()
    {
        rpiView = nullptr;

        if (lightHandle.IsValid())
        {
            CE::Scene* scene = GetScene();
            if (!scene)
                return;

            RPI::Scene* rpiScene = scene->GetRpiScene();
            if (!rpiScene)
                return;

            DirectionalLightFeatureProcessor* fp = rpiScene->GetFeatureProcessor<RPI::DirectionalLightFeatureProcessor>();
            if (!fp)
                return;

            fp->ReleaseLight(lightHandle);
        }
    }

    void DirectionalLightComponent::OnEnabled()
    {
	    Super::OnEnabled();

        if (lightHandle.IsValid())
        {
            //lightHandle->flags.visible = true;
        }
    }

    void DirectionalLightComponent::OnDisabled()
    {
	    Super::OnDisabled();

        if (lightHandle.IsValid())
        {
            CE::Scene* scene = GetScene();
            if (!scene)
                return;

            RPI::Scene* rpiScene = scene->GetRpiScene();
            if (!rpiScene)
                return;

            DirectionalLightFeatureProcessor* fp = rpiScene->GetFeatureProcessor<RPI::DirectionalLightFeatureProcessor>();
            if (!fp)
                return;

            fp->ReleaseLight(lightHandle);
            //lightHandle->flags.visible = false;
        }
    }

    void DirectionalLightComponent::Tick(f32 delta)
    {
	    Super::Tick(delta);

        CE::Scene* scene = GetScene();
        if (!scene)
            return;

        RPI::Scene* rpiScene = scene->GetRpiScene();
        if (!rpiScene)
            return;

        DirectionalLightFeatureProcessor* fp = rpiScene->GetFeatureProcessor<DirectionalLightFeatureProcessor>();
        if (!fp)
            return;

        CameraComponent* mainCamera = scene->GetMainCamera();
        if (!mainCamera)
            return;

        CE::RenderPipeline* rp = mainCamera->GetRenderPipeline();
        if (!rp)
            return;

        if (!lightHandle.IsValid())
        {
            RPI::DirectionalLightHandleDescriptor desc{};
            lightHandle = fp->AcquireLight(desc);
        }

        Vec3 forward = GetForwardVector();
        Vec3 up = GetUpwardVector();

        lightHandle->view = rpiView;
        lightHandle->flags.shadows = enableShadows;

        lightHandle->colorAndIntensity = lightColor.ToVec4();
        lightHandle->colorAndIntensity.w = intensity;
        lightHandle->temperature = temperature;
        lightHandle->shadowDistance = shadowDistance;
        lightHandle->pixelResolution = Vec2i(1, 1) * rp->directionalShadowResolution;

        lightHandle->direction = forward;
        lightHandle->viewPosition = mainCamera->GetPosition() + Vec4(0, 5, 0, 0); // Position light 5 units above camera
        lightHandle->projectionMatrix = Matrix4x4::OrthographicProjection(-shadowDistance, shadowDistance, shadowDistance, -shadowDistance, 1.0f, 100.0f);
        lightHandle->viewMatrix = Matrix4x4::Translation(-lightHandle->viewPosition) *
            Quat::LookRotation(lightHandle->direction).ToMatrix();
        lightHandle->viewProjectionMatrix = lightHandle->projectionMatrix * lightHandle->viewMatrix;

    }

} // namespace CE
