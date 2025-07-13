#pragma once

namespace CE
{
	class RenderPipeline;
	
	ENUM()
	enum class CameraProjection
	{
		Perspective = 0,
		Orthogonal = 1,
	};
	ENUM_CLASS(CameraProjection);

	ENUM()
	enum class CameraType
	{
		MainCamera = 0,
		SecondaryCamera
	};
	ENUM_CLASS(CameraType);

    CLASS()
	class ENGINE_API CameraComponent : public SceneComponent
	{
		CE_CLASS(CameraComponent, SceneComponent)
	public:

		CameraComponent();

		virtual ~CameraComponent();

		RPI::View* GetRpiView() const { return rpiView; }
    	
    	void SetRenderPipeline(CE::RenderPipeline* renderPipeline);
    	
		CE::RenderPipeline* GetRenderPipeline() const { return renderPipeline; }
    	
    protected:

		void OnFieldChanged(const Name& fieldName) override;

		void Tick(f32 delta) override;

		void TickCamera();

		void OnEnabled() override;

		void OnDisabled() override;

	private:

    	FIELD()
    	CameraType cameraType = CameraType::MainCamera;

		FIELD(EditAnywhere, Category = "Camera")
		CameraProjection projection = CameraProjection::Perspective;

		FIELD(EditAnywhere, Category = "Camera")
		float nearPlane = 0.1f;

		FIELD(EditAnywhere, Category = "Camera")
		float farPlane = 1000;

		FIELD(EditAnywhere, Category = "Camera", RangeMin = 10, RangeMax = 120)
		float fieldOfView = 60;
    
		FIELD()
		Vec2 viewport = Vec2(1, 1);

		FIELD()
		Vec2i windowSize = Vec2i(0, 0);

    	FIELD()
    	CE::RenderPipeline* renderPipeline = nullptr;

		RPI::ViewPtr rpiView = nullptr;

	public: // - Accessors -

		CE_PROPERTY(CameraType, cameraType);
		CE_PROPERTY(Projection, projection);
		CE_PROPERTY(NearPlane, nearPlane);
		CE_PROPERTY(FarPlane, farPlane);
		CE_PROPERTY(FieldOfView, fieldOfView);
		CE_PROPERTY(Viewport, viewport);

		friend class CE::Scene;
		friend class RendererSubsystem;
	};

} // namespace CE

#include "CameraComponent.rtti.h"