#pragma once

namespace CE
{
	CLASS(Abstract)
	class ENGINE_API GeometryComponent : public SceneComponent
	{
		CE_CLASS(GeometryComponent, SceneComponent)
	public:

		GeometryComponent();

	protected:

		void OnAttachedToScene(Ref<CE::Scene> scene) override;

		void UpdatePhysicsBody();

	private:

		FIELD()
		Array<Ref<PhysicsBody>> collisionBodies;

		FIELD(EditAnywhere, Category = "Physics")
		bool simulatePhysics = false;

		FIELD(EditAnywhere, Category = "Physics")
		PhysicsMotionType motionType = PhysicsMotionType::Static;



	};
    
} // namespace CE

#include "GeometryComponent.rtti.h"
