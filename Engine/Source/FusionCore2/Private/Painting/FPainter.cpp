#include "FusionCore.h"

namespace CE
{
	FPainter::FPainter(FLayer* layer) : layer(layer)
	{
		drawList = &layer->drawList;
	}

	void FPainter::PushTransform(const FAffineTransform& transform)
	{
		drawList->PushTransform(transform);
	}

	void FPainter::PopTransform()
	{
		drawList->PopTransform();
	}

	FAffineTransform FPainter::GetCurrentTransform()
	{
		return drawList->GetCurrentTransform();
	}

	void FPainter::PathInsert(Vec2 point)
	{
		ZoneScoped;

		path.Insert(point);

		PathMinMax(point);
	}

	void FPainter::PathMinMax(Vec2 point)
	{
		ZoneScoped;

		pathMin.x = Math::Min(point.x, pathMin.x);
		pathMin.y = Math::Min(point.y, pathMin.y);

		pathMax.x = Math::Max(point.x, pathMax.x);
		pathMax.y = Math::Max(point.y, pathMax.y);
	}
} // namespace CE

