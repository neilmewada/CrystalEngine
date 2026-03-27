#pragma once

namespace CE
{
	ENUM()
	enum class FGradientType : u8
	{
		Linear,
		Radial,
		Conic
	};
	ENUM_CLASS(FGradientType);

	STRUCT()
	struct FUSIONCORE_API FGradientKey final
	{
		CE_STRUCT(FGradientKey)
	public:

		FGradientKey() {}

		FGradientKey(f32 position, const Color& color) : position(position), color(color)
		{
		}

		// Normalized position along the gradient (0.0 = start, 1.0 = end)
		FIELD()
		f32 position = 0;

		FIELD()
		Color color;

		SIZE_T GetHash() const;

	};

	STRUCT()
	struct FUSIONCORE_API FGradient final
	{
		CE_STRUCT(FGradient)
	public:

		FIELD()
		Array<FGradientKey> stops;

		// Angle in radians. Used by Linear (direction) and Conic (start angle).
		// Radial gradients ignore this field — center/radius are inferred from the widget rect.
		FIELD()
		float angle = 0;

		FIELD()
		FGradientType gradientType = FGradientType::Linear;

		Self& AddKey(f32 position, const Color& color)
		{
			stops.EmplaceBack(position, color);
			return *this;
		}

		SIZE_T GetHash() const;

		bool IsValid() const
		{
			return stops.GetSize() >= 2;
		}

		bool operator==(const FGradient& rhs) const;

		bool operator!=(const FGradient& rhs) const
		{
			return !operator==(rhs);
		}

	};

}

#include "FGradient.rtti.h"
