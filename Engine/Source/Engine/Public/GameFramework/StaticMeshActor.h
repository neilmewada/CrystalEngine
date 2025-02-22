#pragma once

namespace CE
{
	CLASS()
	class ENGINE_API StaticMeshActor : public Actor
	{
		CE_CLASS(StaticMeshActor, Actor)
	public:

		StaticMeshActor();

		StaticMeshComponent* GetMeshComponent() const { return meshComponent; }

	protected:

		FIELD()
		StaticMeshComponent* meshComponent = nullptr;

	};
    
} // namespace CE

#include "StaticMeshActor.rtti.h"