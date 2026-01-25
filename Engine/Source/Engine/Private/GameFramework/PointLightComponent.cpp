#include "Engine.h"

namespace CE
{

    PointLightComponent::PointLightComponent()
    {

    }

    void PointLightComponent::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (lightHandle.IsValid())
        {
            Ref<CE::Scene> scene = GetScene();
            if (!scene)
                return;

            RPI::Scene* rpiScene = scene->GetRpiScene();
            if (!rpiScene)
                return;

            LocalLightFeatureProcessor* fp = rpiScene->GetFeatureProcessor<RPI::LocalLightFeatureProcessor>();
            if (!fp)
                return;

            fp->ReleaseLight(lightHandle);
        }
    }

    void PointLightComponent::Tick(f32 delta)
    {
	    Super::Tick(delta);

        Ref<CE::Scene> scene = GetScene();
        if (!scene)
            return;

        RPI::Scene* rpiScene = scene->GetRpiScene();
        if (!rpiScene)
            return;

        LocalLightFeatureProcessor* fp = rpiScene->GetFeatureProcessor<LocalLightFeatureProcessor>();
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
            RPI::LocalLightHandleDescriptor desc{};
            lightHandle = fp->AcquireLight(desc);
        }

        Vec3 lookDir = GetForwardVector();
        Vec3 upDir = GetUpwardVector();

        lightHandle->viewPosition = GetPosition();
        lightHandle->viewMatrix = Quat::LookRotation2(lookDir, upDir).ToMatrix() * Matrix4x4::Translation(-GetPosition());

        lightHandle->lightType = LocalLightType::Point;

        lightHandle->range = range;
        lightHandle->temperature = temperature;
        lightHandle->colorAndIntensity = Vec4(lightColor.r, lightColor.g, lightColor.b, intensity);
        lightHandle->worldPos = GetPosition();
    }

    void PointLightComponent::OnFieldChanged(const Name& fieldName)
    {
	    Super::OnFieldChanged(fieldName);

        thread_local const HashSet<Name> lightProperties = {
			NAMEOF(range), NAMEOF(temperature), NAMEOF(lightColor), NAMEOF(intensity)
        };

        if (lightProperties.Exists(fieldName) && lightHandle.IsValid())
        {
            lightHandle->range = range;
            lightHandle->temperature = temperature;
            lightHandle->colorAndIntensity = Vec4(lightColor.r, lightColor.g, lightColor.b, intensity);
        }
    }

    void PointLightComponent::OnFieldEdited(const Name& fieldName)
    {
        Super::OnFieldEdited(fieldName);

        thread_local const HashSet<Name> lightProperties = {
            NAMEOF(range), NAMEOF(temperature), NAMEOF(lightColor), NAMEOF(intensity)
        };

        if (lightProperties.Exists(fieldName) && lightHandle.IsValid())
        {
            lightHandle->range = range;
            lightHandle->temperature = temperature;
            lightHandle->colorAndIntensity = Vec4(lightColor.r, lightColor.g, lightColor.b, intensity);
        }
    }

    void PointLightComponent::OnEnabled()
    {
	    Super::OnEnabled();

        if (lightHandle.IsValid())
        {
			lightHandle->flags.visible = true;
        }
    }

    void PointLightComponent::OnDisabled()
    {
	    Super::OnDisabled();

        if (lightHandle.IsValid())
        {
            lightHandle->flags.visible = false;
        }
    }

} // namespace CE

