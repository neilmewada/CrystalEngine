#include "Engine.h"

namespace CE
{

    CameraComponent::CameraComponent()
    {
		canTick = true;

        //if (!IsDefaultInstance())
        {
#if PLATFORM_DESKTOP
            renderPipeline = CreateDefaultSubobject<MainRenderPipeline>("RenderPipeline");
#else
#error Unimplemented render pipeline
#endif

            rpiView = RPI::View::CreateView("Camera", RPI::View::UsageCamera);

            renderPipeline->SetOwnerCamera(this);
        }
    }

    CameraComponent::~CameraComponent()
    {
        rpiView = nullptr;
    }

    void CameraComponent::SetRenderPipeline(CE::RenderPipeline* renderPipeline)
    {
        Ref<CE::Scene> scene = GetScene();
        if (scene)
        {
            scene->RemoveRenderPipeline(this->renderPipeline.Get());
        }
        
        this->renderPipeline = renderPipeline;
        if (renderPipeline)
        {
            renderPipeline->SetMainViewTag("MainCamera");
        }
        
        if (scene)
        {
            scene->AddRenderPipeline(renderPipeline, this);
        }
    }

    void CameraComponent::OnFieldChanged(const Name& fieldName)
    {
	    Super::OnFieldChanged(fieldName);

        
    }

    void CameraComponent::OnFieldEdited(const Name& fieldName)
    {
	    Super::OnFieldEdited(fieldName);

    }

    void CameraComponent::Tick(f32 delta)
    {
	    Super::Tick(delta);
      
        
    }

    void CameraComponent::TickCamera()
    {
        if (windowSize.x > 0 && windowSize.y > 0)
        {
            RPI::PerViewConstants& viewConstants = rpiView->GetViewConstants();

            if (projection == CameraProjection::Perspective && windowSize.height > 0)
            {
                viewConstants.projectionMatrix = Matrix4x4::PerspectiveProjection((f32)windowSize.width / windowSize.height, fieldOfView, nearPlane, farPlane);
            }
            else if (projection == CameraProjection::Orthogonal && windowSize.height > 0)
            {
                viewConstants.projectionMatrix = Matrix4x4::OrthographicProjection((f32)windowSize.width / windowSize.height, nearPlane, farPlane);
            }

            Vec3 lookDir = GetForwardVector();
            Vec3 upDir = GetUpwardVector();

            viewConstants.viewMatrix = Quat::LookRotation2(lookDir, upDir).ToMatrix() * Matrix4x4::Translation(-GetPosition());
            viewConstants.viewPosition = GetPosition();
            viewConstants.pixelResolution = windowSize.ToVec2();
            viewConstants.viewProjectionMatrix = viewConstants.projectionMatrix * viewConstants.viewMatrix;
			viewConstants.nearPlane = nearPlane;
            viewConstants.farPlane = farPlane;
        }
    }

    void CameraComponent::OnEnabled()
    {
        Super::OnEnabled();

        rpiView->SetEnabled(true);
    }

    void CameraComponent::OnDisabled()
    {
	    Super::OnDisabled();

        rpiView->SetEnabled(false);
    }
} // namespace CE
