#include "Engine.h"

namespace CE
{

	GeometryComponent::GeometryComponent()
	{
		canTick = true;
	}

	void GeometryComponent::OnAttachedToScene(Ref<CE::Scene> scene)
	{
		Super::OnAttachedToScene(scene);

		if (Ref<PhysicsScene> physicsScene = scene->GetPhysicsScene())
		{
			
		}
	}

	void GeometryComponent::UpdatePhysicsBody()
	{

	}

} // namespace CE
