#include "FusionCore.h"


namespace CE
{
	// Credit: Dear ImGui
	static constexpr f32 MinCircleRadius = 0.1f;

	static Vec2 ImBezierCubicCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, float t)
	{
		float u = 1.0f - t;
		float w1 = u * u * u;
		float w2 = 3 * u * u * t;
		float w3 = 3 * u * t * t;
		float w4 = t * t * t;
		return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
	}

	static Vec2 ImBezierQuadraticCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, float t)
	{
		float u = 1.0f - t;
		float w1 = u * u;
		float w2 = 2 * u * t;
		float w3 = t * t;
		return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y);
	}

	static void PathBezierCubicCurveToCasteljau(FPainter* painter,
		float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
	{
		float dx = x4 - x1;
		float dy = y4 - y1;
		float d2 = (x2 - x4) * dy - (y2 - y4) * dx;
		float d3 = (x3 - x4) * dy - (y3 - y4) * dx;
		d2 = (d2 >= 0) ? d2 : -d2;
		d3 = (d3 >= 0) ? d3 : -d3;
		if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
		{
			painter->PathInsert(Vec2(x4, y4));
		}
		else if (level < 10)
		{
			float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
			float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
			float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
			float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
			float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
			float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
			PathBezierCubicCurveToCasteljau(painter, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
			PathBezierCubicCurveToCasteljau(painter, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
		}
	}

	static void PathBezierQuadraticCurveToCasteljau(FPainter* painter,
		float x1, float y1, float x2, float y2, float x3, float y3, float tess_tol, int level)
	{
		float dx = x3 - x1, dy = y3 - y1;
		float det = (x2 - x3) * dy - (y2 - y3) * dx;
		if (det * det * 4.0f < tess_tol * (dx * dx + dy * dy))
		{
			painter->PathInsert(Vec2(x3, y3));
		}
		else if (level < 10)
		{
			float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
			float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
			float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
			PathBezierQuadraticCurveToCasteljau(painter, x1, y1, x12, y12, x123, y123, tess_tol, level + 1);
			PathBezierQuadraticCurveToCasteljau(painter, x123, y123, x23, y23, x3, y3, tess_tol, level + 1);
		}
	}

	// -----------------------------------------------------------------------
	// FPainter
	// -----------------------------------------------------------------------

	FPainter::FPainter(FLayer* layer) : layer(layer)
	{
		drawList = &layer->drawList;

		drawList->fringeScale = 1.0f;

		for (int i = 0; i < ArcFastTableSize; i++)
		{
			const float a = ((float)i * 2 * M_PI) / (float)ArcFastTableSize;
			arcFastVertex[i] = Vec2(cosf(a), sinf(a));
		}

		arcFastRadiusCutoff = ((circleSegmentMaxError) / (1 - cosf(Math::PI / Math::Max((float)(ArcFastTableSize), Math::PI))));
	}

	void FPainter::PushTransform(const FAffineTransform& transform)
	{
		transformStack.Insert(GetCurrentTransform() * transform);
	}

	void FPainter::PopTransform()
	{
		transformStack.RemoveLast();
	}

	FAffineTransform FPainter::GetCurrentTransform()
	{
		return transformStack.IsEmpty() ? FAffineTransform::Identity() : transformStack.Last();
	}

	void FPainter::PathClear()
	{
		ZoneScoped;

		path.RemoveAll();

		pathMin = Vec2(NumericLimits<f32>::Max(), NumericLimits<f32>::Max());
		pathMax = Vec2(NumericLimits<f32>::Min(), NumericLimits<f32>::Min());
	}

	void FPainter::PathInsert(Vec2 point)
	{
		ZoneScoped;

		point = GetCurrentTransform().TransformPoint(point);

		path.Insert(point);

		PathMinMax(point);
	}

	void FPainter::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle)
	{
		ZoneScoped;

		if (radius < MinCircleRadius)
		{
			PathInsert(center);
			return;
		}

		const float arcLength = Math::Abs(endAngle - startAngle);
		const int circleSegmentCount = CalculateNumCircleSegments(radius);
		const int arcSegmentCount = Math::Max(Math::RoundToInt(ceilf(circleSegmentCount * arcLength / (Math::PI * 2.0f))),
			Math::RoundToInt(2.0f * Math::PI / arcLength));

		PathArcTo(center, radius, startAngle, endAngle, arcSegmentCount);
	}

	void FPainter::PathArcToFast(const Vec2& center, float radius, float startAngle, float endAngle)
	{
		ZoneScoped;

		if (radius < MinCircleRadius)
		{
			PathInsert(center);
			return;
		}

		int sampleMin = (int)Math::Round(startAngle / (Math::PI * 2.0f) * ArcFastTableSize);
		int sampleMax = (int)Math::Round(endAngle / (Math::PI * 2.0f) * ArcFastTableSize);

		PathArcToFastInternal(center, radius, sampleMin, sampleMax, 0);
	}

	void FPainter::PathBezierCubicCurveTo(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, int numSegments)
	{
		if (numSegments == 0)
		{
			// Auto-tessellated
			PathBezierCubicCurveToCasteljau(this, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, curveTessellationTolerance, 0);
		}
		else
		{
			float totalSteps = 1.0f / (float)numSegments;
			for (int i = 1; i <= numSegments; i++)
			{
				PathInsert(ImBezierCubicCalc(p1, p2, p3, p4, totalSteps * i));
			}
		}
	}

	void FPainter::PathQuadraticCubicCurveTo(const Vec2& p1, const Vec2& p2, const Vec2& p3, int numSegments)
	{
		if (numSegments == 0)
		{
			// Auto-tessellated
			PathBezierQuadraticCurveToCasteljau(this, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, curveTessellationTolerance, 0);
		}
		else
		{
			float totalSteps = 1.0f / (float)numSegments;
			for (int i = 1; i <= numSegments; i++)
			{
				PathInsert(ImBezierQuadraticCalc(p1, p2, p3, totalSteps * i));
			}
		}
	}

	void FPainter::PathRect(const Rect& rect, const Vec4& cornerRadius)
	{
		const Vec2& min = rect.min;
		const Vec2& max = rect.max;

		if (cornerRadius.GetMax() < 0.5f)
		{
			PathInsert(min);
			PathInsert(Vec2(max.x, min.y));
			PathInsert(max);
			PathInsert(Vec2(min.x, max.y));
		}
		else
		{
			PathArcToFast(Vec2(min.x + cornerRadius.topLeft, min.y + cornerRadius.topLeft),
				cornerRadius.topLeft, Math::PI, Math::PI * 1.5f);
			PathArcToFast(Vec2(max.x - cornerRadius.topRight, min.y + cornerRadius.topRight),
				cornerRadius.topRight, Math::PI * 1.5f, Math::PI * 2.0f);
			PathArcToFast(Vec2(max.x - cornerRadius.bottomRight, max.y - cornerRadius.bottomRight),
				cornerRadius.bottomRight, 0.0f, Math::PI * 0.5f);
			PathArcToFast(Vec2(min.x + cornerRadius.bottomLeft, max.y - cornerRadius.bottomLeft),
				cornerRadius.bottomLeft, Math::PI * 0.5f, Math::PI);
		}
	}

	bool FPainter::PathFill(bool antiAliased)
	{
		bool result = PathFillInternal(antiAliased);
		PathClear();
		return result;
	}

	bool FPainter::PathStroke(bool closed, bool antiAliased)
	{
		bool result = PathStrokeInternal(closed, antiAliased);
		PathClear();
		return result;
	}

	bool FPainter::PathFillAndStroke(bool antiAliased)
	{
		bool result = PathFillInternal(antiAliased);
		result |= PathStrokeInternal(true, antiAliased);
		PathClear();
		return result;
	}

	void FPainter::StrokeRect(const Rect& rect, const Vec4& cornerRadius, bool antiAliased)
	{
		PathClear();
		PathRect(rect, cornerRadius);
		PathStroke(true, antiAliased);
	}

	void FPainter::FillRect(const Rect& rect, const Vec4& cornerRadius, bool antiAliased)
	{
		PathClear();
		PathRect(rect, cornerRadius);
		PathFill(antiAliased);
	}

	void FPainter::FillAndStrokeRect(const Rect& rect, const Vec4& cornerRadius, bool antiAliased)
	{
		PathClear();
		PathRect(rect, cornerRadius);
		PathFillAndStroke(antiAliased);
	}

	void FPainter::StrokeCircle(const Vec2& center, f32 radius, bool antiAliased)
	{
		PathClear();
		PathArcToFast(center, radius, 0, Math::PI * 2.0f);
		PathStroke(true, antiAliased);
	}

	void FPainter::FillCircle(const Vec2& center, f32 radius, bool antiAliased)
	{
		PathClear();
		PathArcToFast(center, radius, 0, Math::PI * 2.0f);
		PathFill(antiAliased);
	}

	void FPainter::FillAndStrokeCircle(const Vec2& center, f32 radius, bool antiAliased)
	{
		PathClear();
		PathArcToFast(center, radius, 0, Math::PI * 2.0f);
		PathFillAndStroke(antiAliased);
	}

	void FPainter::DrawLine(const Vec2& p1, const Vec2& p2, bool antiAliased)
	{
		PathClear();
		PathInsert(p1);
		PathInsert(p2);
		PathStroke(false, antiAliased);
	}

	void FPainter::StrokeShape(const Rect& rect, const FShape& shape, bool antiAliased)
	{
		switch (shape.GetShapeType())
		{
		case FShapeType::None:
			return;
		case FShapeType::Rect:
			StrokeRect(rect, Vec4(), antiAliased);
			break;
		case FShapeType::RoundedRect:
			StrokeRect(rect, shape.GetCornerRadius(), antiAliased);
			break;
		case FShapeType::Circle:
			{
				Vec2 center = rect.min + rect.GetSize() / 2.0f;
				f32 radius = rect.GetSize().GetMin() / 2.0f;
				StrokeCircle(center, radius, antiAliased);
			}
			break;
		}
	}

	void FPainter::FillShape(const Rect& rect, const FShape& shape, bool antiAliased)
	{
		switch (shape.GetShapeType())
		{
		case FShapeType::None:
			return;
		case FShapeType::Rect:
			FillRect(rect, Vec4(), antiAliased);
			break;
		case FShapeType::RoundedRect:
			FillRect(rect, shape.GetCornerRadius(), antiAliased);
			break;
		case FShapeType::Circle:
			{
				Vec2 center = rect.min + rect.GetSize() / 2.0f;
				f32 radius = rect.GetSize().GetMin() / 2.0f;
				FillCircle(center, radius, antiAliased);
			}
			break;
		}
	}

	void FPainter::FillAndStrokeShape(const Rect& rect, const FShape& shape, bool antiAliased)
	{
		switch (shape.GetShapeType())
		{
		case FShapeType::None:
			return;
		case FShapeType::Rect:
			FillAndStrokeRect(rect, Vec4(), antiAliased);
			break;
		case FShapeType::RoundedRect:
			FillAndStrokeRect(rect, shape.GetCornerRadius(), antiAliased);
			break;
		case FShapeType::Circle:
		{
			Vec2 center = rect.min + rect.GetSize() / 2.0f;
			f32 radius = rect.GetSize().GetMin() / 2.0f;
			FillAndStrokeCircle(center, radius, antiAliased);
		}
		break;
		}
	}

	void FPainter::PushClip(const Rect& rect, const FShape& shape)
	{
		int index = drawList->clipRectArray.GetCount();

		clipStack.Insert(index);

		Vec4 radii = shape.GetCornerRadius();
		Vec4 halfSize = rect.GetSize() / 2.0f;

		if (shape.GetShapeType() == FShapeType::Rect)
		{
			radii = Vec4();
		}
		else if (shape.GetShapeType() == FShapeType::Circle)
		{
			radii = Vec4();
			f32 min = halfSize.GetMin();
			halfSize = Vec2(min, min);
		}

		Vec2 clipCenter = rect.min + halfSize;

		drawList->clipRectArray.Insert(FUIClipRect{
			.clipInverseTransform = (GetCurrentTransform() * FAffineTransform::Translation(clipCenter)).ToMatrix4x4().GetInverse(),
			.cornerRadii = radii,
			.clipHalfSize = halfSize
		});
	}

	void FPainter::PopClip()
	{
		clipStack.RemoveLast();
	}

	int FPainter::CalculateNumCircleSegments(float radius) const
	{
		const int radiusIndex = (int)(radius + 0.999999f); // ceil to never reduce accuracy
		return Math::Clamp((((((int)ceilf(Math::PI / acosf(1 - Math::Min((circleSegmentMaxError), (radius)) / (radius)))) + 1) / 2) * 2), 4, 512);
	}

	void FPainter::PathMinMax(Vec2 point)
	{
		ZoneScoped;

		pathMin.x = Math::Min(point.x, pathMin.x);
		pathMin.y = Math::Min(point.y, pathMin.y);

		pathMax.x = Math::Max(point.x, pathMax.x);
		pathMax.y = Math::Max(point.y, pathMax.y);
	}

	void FPainter::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, int numSegments)
	{
		if (radius < 0.5f)
		{
			PathInsert(center);
			return;
		}

		for (int i = 0; i <= numSegments; ++i)
		{
			const float angle = startAngle + ((float)i / (float)numSegments) * (endAngle - startAngle);
			PathInsert(Vec2(center.x + Math::Cos(angle) * radius, center.y + Math::Sin(angle) * radius));
		}
	}

	void FPainter::PathArcToFastInternal(const Vec2& center, float radius, int sampleMin, int sampleMax, int step)
	{
		if (radius < MinCircleRadius)
		{
			PathInsert(center);
			return;
		}

		// Calculate arc auto segment step size
		if (step <= 0)
			step = ArcFastTableSize / CalculateNumCircleSegments(radius);

		// Make sure we never do steps larger than one quarter of the circle
		step = Math::Clamp(step, 1, (int)ArcFastTableSize / 4);

		const int sampleRange = abs(sampleMax - sampleMin);
		const int nextStep = step;

		int samples = sampleRange + 1;
		bool extraMaxSample = false;
		if (step > 1)
		{
			samples = sampleRange / step + 1;
			const int overstep = sampleRange % step;

			if (overstep > 0)
			{
				extraMaxSample = true;
				samples++;

				// When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
				// distribute first step range evenly between them by reducing first step size.
				if (sampleRange > 0)
					step -= (step - overstep) / 2;
			}
		}

		path.InsertRange(samples, Vec2());

		Vec2* outPtr = path.GetData() + (path.GetCount() - samples);

		int sampleIndex = sampleMin;
		if (sampleIndex < 0 || sampleIndex >= ArcFastTableSize)
		{
			sampleIndex = sampleIndex % ArcFastTableSize;
			if (sampleIndex < 0)
				sampleIndex += ArcFastTableSize;
		}

		if (sampleMax >= sampleMin)
		{
			for (int a = sampleMin; a <= sampleMax; a += step, sampleIndex += step, step = nextStep)
			{
				if (sampleIndex >= ArcFastTableSize)
					sampleIndex -= ArcFastTableSize;

				const Vec2 s = arcFastVertex[sampleIndex];
				outPtr->x = center.x + s.x * radius;
				outPtr->y = center.y + s.y * radius;
				*outPtr = GetCurrentTransform().TransformPoint(*outPtr);
				PathMinMax(*outPtr);
				outPtr++;
			}
		}
		else
		{
			for (int a = sampleMin; a >= sampleMax; a -= step, sampleIndex -= step, step = nextStep)
			{
				if (sampleIndex < 0)
					sampleIndex += ArcFastTableSize;

				const Vec2 s = arcFastVertex[sampleIndex];
				outPtr->x = center.x + s.x * radius;
				outPtr->y = center.y + s.y * radius;
				*outPtr = GetCurrentTransform().TransformPoint(*outPtr);
				PathMinMax(*outPtr);
				outPtr++;
			}
		}

		if (extraMaxSample)
		{
			int normalizedMaxSample = sampleMax % ArcFastTableSize;
			if (normalizedMaxSample < 0)
				normalizedMaxSample += ArcFastTableSize;

			const Vec2 s = arcFastVertex[normalizedMaxSample];
			outPtr->x = center.x + s.x * radius;
			outPtr->y = center.y + s.y * radius;
			*outPtr = GetCurrentTransform().TransformPoint(*outPtr);
			PathMinMax(*outPtr);
			outPtr++;
		}
	}

	bool FPainter::PathStrokeInternal(bool closed, bool antiAliased)
	{
		if (!currentPen.IsValidPen())
			return true;

		ZoneScoped;

		f32 thickness = currentPen.GetThickness();

		Color penColor = currentPen.GetColor();
		penColor.a *= GetCurrentOpacity();

		u32 color = penColor.ToU32();

		FUIDrawItem drawItem{};
		drawItem.clipRectIndex = GetCurrentClipIndex();

		if (currentPen.HasGradient())
		{
			const FGradient& g = currentPen.GetGradient();
			drawItem.shaderType = FUIShaderType::LinearGradient;
			drawItem.gradientStartIndex = drawList->gradientStopArray.GetCount();
			drawItem.gradientStopCount = g.stops.GetSize();

			for (const auto& stop : g.stops)
			{
				drawList->gradientStopArray.Insert({ .packedColor = stop.color.ToU32(), .position = stop.position });
			}

			drawItem.data[0] = 0.0f; // angle=0: shader computes gradientT = uv.x directly
		}
		else
		{
			drawItem.shaderType = FUIShaderType::SolidColor;
		}

		u32 drawItemIndex = drawList->AddDrawItem(drawItem);

		const bool hasGradient = currentPen.HasGradient();

		if (currentPen.GetStyle() == FPenStyle::Solid && closed && hasGradient)
		{
			path.Insert(path[0]);
		}

		// Pre-compute per-point t values (0→1 along path length) for gradient pens
		tempPoints.RemoveAll();
		if (currentPen.HasGradient())
		{
			const int numPoints = (int)path.GetCount();
			const int segCount  = closed ? numPoints : numPoints - 1;

			f32 totalLength = 0;
			for (int i = 0; i < segCount; i++)
				totalLength += Vec2::Distance(path[i], path[(i + 1) % numPoints]);

			const f32 gradientOffset = currentPen.GetGradientOffset();

			tempPoints.InsertRange(numPoints, 0.0f);
			tempPoints[0] = gradientOffset;
			f32 acc = 0;

			for (int i = 0; i < segCount; i++)
			{
				const int j = (i + 1) % numPoints;
				acc += Vec2::Distance(path[i], path[j]);
				if (j != 0)
					tempPoints[j] = totalLength > 0 ? acc / totalLength + gradientOffset : 0.0f;
			}
		}

		switch (currentPen.GetStyle())
		{
		case FPenStyle::None:
			return true;
		case FPenStyle::Solid:
			// Only do closed line if we are not rendering a gradient
			drawList->AddPolyLine(path.GetData(), (int)path.GetCount(), color, thickness, closed && !hasGradient, antiAliased, drawItemIndex,
				hasGradient ? tempPoints.GetData() : nullptr);
			break;
		case FPenStyle::Dashed:
		{
			const f32 dashLen = currentPen.GetDashLength();
			const f32 gapLen  = currentPen.GetDashGap();

			f32  dashOffset = 0.0f;
			bool inDash     = true;
			currentPen.InitDashState(dashOffset, inDash);

			const int numPoints = (int)path.GetCount();

			for (int i = 0; i < numPoints; ++i)
			{
				if (!closed && i == numPoints - 1)
					break;

				const int  nextIdx = (i + 1) % numPoints;
				const Vec2 p0      = path[i];
				const Vec2 p1      = path[nextIdx];
				const f32  segLen  = Vec2::Distance(p0, p1);

				if (segLen < 0.001f)
					continue;

				const Vec2 dir = (p1 - p0) / segLen;
				const f32  t0  = hasGradient ? tempPoints[i]       : 0.0f;
				const f32  t1  = hasGradient ? tempPoints[nextIdx] : 0.0f;

				f32 segPos = 0.0f;
				while (segPos < segLen - 0.001f)
				{
					const f32 remaining = segLen - segPos;

					if (inDash)
					{
						const f32 drawLen = Math::Min(remaining, dashLen - dashOffset);
						const f32 aFrac   = segPos / segLen;
						const f32 bFrac   = (segPos + drawLen) / segLen;

						Vec2 pts[2] = { p0 + dir * segPos, p0 + dir * (segPos + drawLen) };
						f32  uvs[2] = { t0 + (t1 - t0) * aFrac, t0 + (t1 - t0) * bFrac };

						drawList->AddPolyLine(pts, 2, color, thickness, false, antiAliased, drawItemIndex,
							hasGradient ? uvs : nullptr);

						dashOffset += drawLen;
						segPos     += drawLen;

						if (dashOffset >= dashLen - 0.001f)
						{
							dashOffset = 0.0f;
							inDash     = false;
						}
					}
					else
					{
						const f32 skipLen = Math::Min(remaining, gapLen - dashOffset);
						dashOffset += skipLen;
						segPos     += skipLen;

						if (dashOffset >= gapLen - 0.001f)
						{
							dashOffset = 0.0f;
							inDash     = true;
						}
					}
				}
			}
		}
		break;
		case FPenStyle::Dotted:
		{
			constexpr f32 dotLen = 2.0f;
			const f32     gapLen = currentPen.GetDashGap();

			f32  dashOffset = 0.0f;
			bool inDash     = true;
			currentPen.InitDashState(dashOffset, inDash);

			const int numPoints = (int)path.GetCount();

			for (int i = 0; i < numPoints; ++i)
			{
				if (!closed && i == numPoints - 1)
					break;

				const int  nextIdx = (i + 1) % numPoints;
				const Vec2 p0      = path[i];
				const Vec2 p1      = path[nextIdx];
				const f32  segLen  = Vec2::Distance(p0, p1);

				if (segLen < 0.001f)
					continue;

				const Vec2 dir = (p1 - p0) / segLen;
				const f32  t0  = hasGradient ? tempPoints[i]       : 0.0f;
				const f32  t1  = hasGradient ? tempPoints[nextIdx] : 0.0f;

				f32 segPos = 0.0f;
				while (segPos < segLen - 0.001f)
				{
					const f32 remaining = segLen - segPos;

					if (inDash)
					{
						const f32 drawLen = Math::Min(remaining, dotLen - dashOffset);
						const f32 aFrac   = segPos / segLen;
						const f32 bFrac   = (segPos + drawLen) / segLen;

						Vec2 pts[2] = { p0 + dir * segPos, p0 + dir * (segPos + drawLen) };
						f32  uvs[2] = { t0 + (t1 - t0) * aFrac, t0 + (t1 - t0) * bFrac };

						drawList->AddPolyLine(pts, 2, color, thickness, false, antiAliased, drawItemIndex,
							hasGradient ? uvs : nullptr);

						dashOffset += drawLen;
						segPos     += drawLen;

						if (dashOffset >= dotLen - 0.001f)
						{
							dashOffset = 0.0f;
							inDash     = false;
						}
					}
					else
					{
						const f32 skipLen = Math::Min(remaining, gapLen - dashOffset);
						dashOffset += skipLen;
						segPos     += skipLen;

						if (dashOffset >= gapLen - 0.001f)
						{
							dashOffset = 0.0f;
							inDash     = true;
						}
					}
				}
			}
		}
		break;
		}

		return true;
	}

	bool FPainter::PathFillInternal(bool antiAliased)
	{
		if (path.IsEmpty() || currentBrush.GetBrushStyle() == FBrushStyle::None)
		{
			return true;
		}

		ZoneScoped;

		Rect minMax = Rect(pathMin, pathMax);

		Color brushColor = currentBrush.GetColor();
		brushColor.a *= GetCurrentOpacity();

		u32 color = brushColor.ToU32();

		// First item will always be a SolidFill shader.
		u32 drawItemIndex = 0;

		switch (currentBrush.GetBrushStyle())
		{
		case FBrushStyle::None:
			return true;
		case FBrushStyle::SolidFill:
			{
				FUIDrawItem drawItem{};
				drawItem.clipRectIndex = GetCurrentClipIndex();
				drawItem.shaderType = FUIShaderType::SolidColor;

				drawItemIndex = drawList->AddDrawItem(drawItem);
			}
			break;
		case FBrushStyle::Image:
			{
				FUIDrawItem drawItem{};
				drawItem.clipRectIndex = GetCurrentClipIndex();
				drawItem.shaderType = FUIShaderType::SolidColor;
				drawItem.textureIndex = 0;
				// TODO: Add support for images
				
				drawItemIndex = drawList->AddDrawItem(drawItem);
			}
			break;
		case FBrushStyle::Gradient:
			{
				FUIDrawItem drawItem{};
				drawItem.clipRectIndex = GetCurrentClipIndex();
				drawItem.shaderType = FUIShaderType::LinearGradient;
				drawItem.gradientStartIndex = drawList->gradientStopArray.GetCount();
				drawItem.gradientStopCount = currentBrush.GetGradient().stops.GetSize();

				if (drawItem.gradientStopCount < 2)
					return true;

				for (const auto& stop : currentBrush.GetGradient().stops)
				{
					drawList->gradientStopArray.Insert({ .packedColor = stop.color.ToU32(), .position = stop.position });
				}

				drawItem.data[0] = currentBrush.GetGradient().angle;

				drawItemIndex = drawList->AddDrawItem(drawItem);
			}
			break;
		default:
			return true;
		}

		drawList->AddConvexPolyFilled(path.GetData(), path.GetCount(), color, antiAliased, &minMax, drawItemIndex);

		return true;
	}

	// ------------------------------------------------------------------------
	// Geometry
	// ------------------------------------------------------------------------


} // namespace CE

