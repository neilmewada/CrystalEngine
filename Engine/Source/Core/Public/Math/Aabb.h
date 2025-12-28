#pragma once

namespace CE
{

	class CORE_API Aabb
	{
	public:

		Aabb() = default;

		Aabb(f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ)
			: minX(minX), minY(minY), minZ(minZ), maxX(maxX), maxY(maxY), maxZ(maxZ)
		{}

		Aabb(const Vec3& min, const Vec3& max)
			: minX(min.x), minY(min.y), minZ(min.z), maxX(max.x), maxY(max.y), maxZ(max.z)
		{}

		bool IsInside(const Vec3& point) const
		{
			return point.x >= minX && point.x <= maxX &&
				   point.y >= minY && point.y <= maxY &&
				   point.z >= minZ && point.z <= maxZ;
		}

		f32    minX;
		f32    minY;
		f32    minZ;
		f32    maxX;
		f32    maxY;
		f32    maxZ;
	};

}
