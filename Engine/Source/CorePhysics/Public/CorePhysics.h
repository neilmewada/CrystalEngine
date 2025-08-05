#pragma once

#include "Core.h"

namespace JPH
{
	class PhysicsSystem;
	class Shape;
	class TempAllocator;
	class JobSystemThreadPool;
	class BroadPhaseLayerInterface;
	class ObjectVsBroadPhaseLayerFilter;
	class ObjectLayerPairFilter;
	class Body;
	class ContactListener;
	class PhysicsMaterialImpl;
}

namespace CE
{
	class PhysicsScene;
}

#include "CorePhysics/PhysicsLayers.h"
#include "CorePhysics/PhysicsMaterial.h"
#include "CorePhysics/Body/PhysicsShape.h"
#include "CorePhysics/Body/PhysicsBody.h"
#include "CorePhysics/Scene/PhysicsScene.h"
#include "CorePhysics/PhysicsSystem.h"

