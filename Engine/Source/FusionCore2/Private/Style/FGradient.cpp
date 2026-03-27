#include "FusionCore.h"

namespace CE
{
	SIZE_T FGradientKey::GetHash() const
	{
		return GetCombinedHash(CE::GetHash(position), CE::GetHash(color));
	}

	SIZE_T FGradient::GetHash() const
	{
		SIZE_T hash = CE::GetHash(gradientType);
		for (const auto& stop : stops)
		{
			CombineHash(hash, stop);
		}
		CombineHash(hash, angle);
		return hash;
	}

	bool FGradient::operator==(const FGradient& rhs) const
	{
		if (gradientType != rhs.gradientType)
			return false;
		if (!Math::ApproxEquals(angle, rhs.angle))
			return false;
		if (stops.GetSize() != rhs.stops.GetSize())
			return false;
		for (int i = 0; i < (int)stops.GetSize(); i++)
		{
			if (!Math::ApproxEquals(stops[i].position, rhs.stops[i].position) || stops[i].color != rhs.stops[i].color)
				return false;
		}
		return true;
	}

} // namespace CE
