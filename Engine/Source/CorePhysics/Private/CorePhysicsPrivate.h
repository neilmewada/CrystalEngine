#pragma once

#include "CorePhysics.h"

#include "CorePhysics/PhysicsLimits.inl"

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/ObjectStream/SerializableAttributeTyped.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/CompoundShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace JPH
{
	class PhysicsMaterialImpl : public JPH::PhysicsMaterial
	{
	public:

		PhysicsMaterialImpl() = default;

		PhysicsMaterialImpl(CE::Ref<CE::PhysicsMaterial> material)
		{
			staticFriction = material->staticFriction;
			dynamicFriction = material->dynamicFriction;
			bounciness = material->bounciness;
			frictionCombine = (int)material->frictionCombine;
			bouncinessCombine = (int)material->bouncinessCombine;
		}

		float staticFriction = 0.5f;
		float dynamicFriction = 0.5f;
		float bounciness = 0.0f;
		int frictionCombine = (int)CE::PhysicsCombineMode::Average;
		int bouncinessCombine = (int)CE::PhysicsCombineMode::Average;

		static float Combine(float a, float b, int mode)
		{
			switch ((CE::PhysicsCombineMode)mode)
			{
			default:
			case CE::PhysicsCombineMode::Average:  return (a + b) * 0.5f;
			case CE::PhysicsCombineMode::Multiply: return a * b;
			case CE::PhysicsCombineMode::Minimum:  return std::min(a, b);
			case CE::PhysicsCombineMode::Maximum:  return std::max(a, b);
			}
		}

	};
}
