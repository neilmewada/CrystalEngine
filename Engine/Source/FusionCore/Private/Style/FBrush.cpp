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

	FBrush::FBrush()
		: fillColor(Colors::Clear)
		, imageName(Name())
		, tiling(FBrushTiling::None)
		, brushStyle(FBrushStyle::None)
		, imageFit(FImageFit::Fill)
	{

	}

	FBrush::FBrush(const Color& fillColor, FBrushStyle brushStyle)
		: fillColor(fillColor)
		, brushStyle(brushStyle)
		, imageFit(FImageFit::Fill)
	{

	}

	FBrush::FBrush(const Name& imageName, const Color& fillColor)
		: fillColor(fillColor)
		, imageName(imageName)
		, tiling(FBrushTiling::None)
		, brushStyle(FBrushStyle::Image)
		, imageFit(FImageFit::Fill)
	{
		
	}

	FBrush::FBrush(const FGradient& gradient, const Color& tintColor)
		: fillColor(Colors::White)
		, gradient(gradient)
		, brushStyle(FBrushStyle::Gradient)
		, imageFit(FImageFit::Fill)
	{
		
	}

	FBrush::~FBrush()
	{
		switch (brushStyle)
		{
		case FBrushStyle::Image:
			imageName.~Name();
			break;
		default:
			break;
		}
	}

	bool FBrush::IsValidBrush()
	{
		switch (brushStyle)
		{
		case FBrushStyle::None:
			return false;
		case FBrushStyle::SolidFill:
			return fillColor.a > 0.001f;
		case FBrushStyle::Image:
			return fillColor.a > 0.001f && imageName.IsValid();
		case FBrushStyle::Gradient:
			return gradient.stops.GetSize() >= 2 && fillColor.a > 0.001f;
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
			return fillColor == rhs.fillColor;
		case FBrushStyle::Gradient:
			return fillColor == rhs.fillColor && gradient == rhs.gradient;
		case FBrushStyle::Image:
			return fillColor == rhs.fillColor && imageName == rhs.imageName;
		case FBrushStyle::None:
			break;
		}

		return true;
	}

	FBrush FBrush::WithTint(const Color& fillColor) const
	{
		FBrush result = *this;
		result.fillColor = fillColor;
		return result;
	}
    
} // namespace CE
