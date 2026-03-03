#include "FusionCore.h"

namespace CE
{

	FShape::FShape(FShapeType shapeType) : shapeType(shapeType)
	{
		
	}

	FShape FShape::Rect()
	{
		return FShapeType::Rect;
	}

	FShape FShape::Circle()
	{
		return FShapeType::Circle;
	}

	FShape FShape::RoundedRect(const Vec4& cornerRadius)
	{
		if (cornerRadius == Vec4())
			return Rect();

		FShape shape = FShape(FShapeType::RoundedRect);
		shape.cornerRadius = cornerRadius;
		return shape;
	}

	bool FShape::operator==(const FShape& rhs) const
	{
		if (shapeType != rhs.shapeType)
			return false;

		if (shapeType == FShapeType::RoundedRect)
			return cornerRadius == rhs.cornerRadius;

		return true;
	}

} // namespace CE
