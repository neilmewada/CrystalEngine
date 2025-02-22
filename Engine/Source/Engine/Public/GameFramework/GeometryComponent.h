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

	};
    
} // namespace CE

#include "GeometryComponent.rtti.h"
