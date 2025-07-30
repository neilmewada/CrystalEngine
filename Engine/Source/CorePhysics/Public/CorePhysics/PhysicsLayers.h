#pragma once

namespace CE
{
	static constexpr u16 MaxPhysicsLayers = 64;

	ENUM()
	enum class BuiltinPhysicsLayer : u16
	{
		Static = 0,
		Dynamic,
		Character,
		UI
	};
	ENUM_CLASS_FLAGS(BuiltinPhysicsLayer);


	struct COREPHYSICS_API PhysicsLayer final
	{
	public:

		constexpr PhysicsLayer() = default;

		constexpr PhysicsLayer(u16 value) : value(value)
		{
		}

		constexpr PhysicsLayer(BuiltinPhysicsLayer layer) : value(static_cast<u16>(layer))
		{
		}

		constexpr bool operator==(const PhysicsLayer& other) const
		{
			return value == other.value;
		}

		constexpr bool operator!=(const PhysicsLayer& other) const
		{
			return value != other.value;
		}

		constexpr bool IsValid() const
		{
			return value != NumericLimits<u16>::Max();
		}

		constexpr operator u16() const
		{
			return value;
		}

		SIZE_T GetHash() const
		{
			return CE::GetHash(value);
		}

	private:

		u16 value = NumericLimits<u16>::Max();
	};

} // namespace CE::Physics

CE_RTTI_POD(COREPHYSICS_API, CE, PhysicsLayer);

#include "PhysicsLayers.rtti.h"