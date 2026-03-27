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

	FBrush::FBrush()
		: color(Colors::Clear)
		, imageName(Name())
		, tiling(FBrushTiling::None)
		, brushStyle(FBrushStyle::None)
		, imageFit(FImageFit::Fill)
	{

	}

	FBrush::FBrush(const Color& fillColor, FBrushStyle brushStyle)
		: color(fillColor)
		, brushStyle(brushStyle)
		, imageFit(FImageFit::Fill)
	{

	}

	FBrush::FBrush(const Name& imageName, const Color& tintColor)
		: color(tintColor)
		, imageName(imageName)
		, tiling(FBrushTiling::None)
		, brushStyle(FBrushStyle::Image)
		, imageFit(FImageFit::Fill)
	{

	}

	FBrush::FBrush(const FGradient& gradient, const Color& tintColor)
		: color(tintColor)
		, gradient(gradient)
		, brushStyle(FBrushStyle::Gradient)
		, imageFit(FImageFit::Fill)
	{

	}

	FBrush::~FBrush()
	{
		
	}

	bool FBrush::IsValidBrush()
	{
		switch (brushStyle)
		{
		case FBrushStyle::None:
			return false;
		case FBrushStyle::SolidFill:
			return color.a > 0.001f;
		case FBrushStyle::Image:
			if (imageFit == FImageFit::NineSlice)
				return color.a > 0.001f && imageName.IsValid() && sliceMargins != FMargin();
			return color.a > 0.001f && imageName.IsValid();
		case FBrushStyle::Gradient:
			return gradient.stops.GetSize() >= 2 && color.a > 0.001f;
		}

		return true;
	}

	bool FBrush::operator==(const FBrush& rhs) const
	{
		if (brushStyle != rhs.brushStyle)
			return false;
		if (tiling != rhs.tiling)
			return false;
		if (imageFit != rhs.imageFit)
			return false;
		if (brushPos != rhs.brushPos)
			return false;
		if (brushSize != rhs.brushSize)
			return false;

		switch (brushStyle)
		{
		case FBrushStyle::SolidFill:
			return color == rhs.color;
		case FBrushStyle::Gradient:
			return color == rhs.color && gradient == rhs.gradient;
		case FBrushStyle::Image:
			return color == rhs.color && imageName == rhs.imageName && (imageFit != FImageFit::NineSlice || sliceMargins == rhs.sliceMargins);
		case FBrushStyle::None:
			break;
		}

		return true;
	}

}
