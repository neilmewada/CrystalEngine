#pragma once

namespace CE
{
	static constexpr u16 MaxPhysicsLayers = 64;

	ENUM()
	enum class BuiltinPhysicsLayer : u16
	{
		Default = 0,
		Character,
		UI,
	};
	ENUM_CLASS_FLAGS(BuiltinPhysicsLayer);


	struct COREPHYSICS_API PhysicsLayer final
	{
	public:
		using ValueType = u16;

		static constexpr ValueType PODVersion = 1;

		constexpr PhysicsLayer() = default;

		constexpr PhysicsLayer(ValueType value) : value(value)
		{
		}

		constexpr PhysicsLayer(BuiltinPhysicsLayer layer) : value(static_cast<ValueType>(layer))
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
			return value != NumericLimits<ValueType>::Max();
		}

		constexpr operator ValueType() const
		{
			return value;
		}

		SIZE_T GetHash() const
		{
			return CE::GetHash(value);
		}

		void SerializePOD(Stream* stream)
		{
			*stream << PODVersion;
			*stream << value;
		}

		void DeserializePOD(Stream* stream)
		{
			ValueType version;
			*stream >> version;
			*stream >> value;
		}

	private:

		ValueType value = NumericLimits<ValueType>::Max();

	};

} // namespace CE::Physics

CE_RTTI_POD(COREPHYSICS_API, CE, PhysicsLayer);

#include "PhysicsLayers.rtti.h"