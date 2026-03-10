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

		// Angle in degrees. Used by Linear (direction) and Conic (start angle).
		// Radial gradients ignore this field — center/radius are inferred from the widget rect.
		FIELD()
		float angle = 0;

		FIELD()
		FGradientType gradientType = FGradientType::Linear;

		Self& AddKey(f32 position, const Color& color)
		{
			stops.Add(FGradientKey(position, color));
			return *this;
		}

		SIZE_T GetHash() const;

		bool IsValid() const
		{
			return stops.GetSize() >= 2;
		}

		bool operator==(const FGradient& rhs) const
		{
			return GetHash() == rhs.GetHash();
		}

		bool operator!=(const FGradient& rhs) const
		{
			return !operator==(rhs);
		}

	};

	// Determines what content the brush renders.
	ENUM()
	enum class FBrushStyle : u8
	{
		None = 0,
		SolidFill,  // Filled with a solid color (uses color)
		Image,      // Textured fill (uses imageName, color as tint, imageFit, brushSize, brushPos)
		Gradient    // Gradient fill (uses gradient, color as tint)
	};
	ENUM_CLASS(FBrushStyle);

	// Controls how an image brush tiles when brushSize is smaller than the widget rect.
	// Only applies to FBrushStyle::Image. Mutually exclusive with NineSlice.
	ENUM()
	enum class FBrushTiling : u8
	{
		None = 0,
		TileX,   // Repeat horizontally
		TileY,   // Repeat vertically
		TileXY   // Repeat in both axes
	};
	ENUM_CLASS(FBrushTiling);

	// Controls how an image brush is fitted into the widget rect.
	// FPainter computes vertex UVs from this at tessellation time.
	ENUM()
	enum class FImageFit : u8
	{
		Fill = 0, // Stretch to fill the widget rect exactly (default, ignores aspect ratio)
		Contain,  // Scale uniformly to fit within the rect, preserving aspect ratio (may letterbox)
		Cover,    // Scale uniformly to fill the rect, preserving aspect ratio (may crop)
		NineSlice // Fixed corners, stretched edges/center. Requires sliceMargins to be set.
	};
	ENUM_CLASS(FImageFit);

	// Defines the fill used to paint the interior of a shape.
	// Supports solid color, image (textured), and gradient fills.
	//
	// Key fields by style:
	//   SolidFill : color
	//   Image     : imageName, color (tint), imageFit, tiling, brushSize, brushPos, sliceMargins (NineSlice only)
	//   Gradient  : gradient, color (tint)
	//
	// brushSize : explicit pixel size of the image within the widget rect. Vec2(-1,-1) = auto (driven by imageFit).
	// brushPos  : normalized anchor point (0,0 = top-left, 0.5,0.5 = center, 1,1 = bottom-right). Like CSS background-position.
	STRUCT()
	struct FUSIONCORE_API FBrush final
	{
		CE_STRUCT(FBrush)
	public:

		FBrush();

		FBrush(const Color& fillColor, FBrushStyle brushStyle = FBrushStyle::SolidFill);

		// Image brush. tintColor is multiplied with the texture color (White = no tint).
		FBrush(const Name& imageName, const Color& tintColor = Colors::White);

		// Gradient brush. tintColor is multiplied with the gradient output (White = no tint).
		FBrush(const FGradient& gradient, const Color& tintColor = Colors::White);

		~FBrush();

		bool IsValidBrush();

		FBrushStyle GetBrushStyle() const { return brushStyle; }
		FBrushTiling GetBrushTiling() const { return tiling; }
		const FGradient& GetGradient() const { return gradient; }

		void SetBrushStyle(FBrushStyle brushStyle) { this->brushStyle = brushStyle; }
		void SetBrushTiling(FBrushTiling tiling) { this->tiling = tiling; }

		// For SolidFill: the fill color. For Image/Gradient: the tint color (multiplied with texture/gradient output).
		const Color& GetColor() const { return color; }
		void SetColor(const Color& color) { this->color = color; }

		const Name& GetImageName() const { return imageName; }

		FImageFit GetImageFit() const { return imageFit; }
		void SetImageFit(FImageFit imageFit) { this->imageFit = imageFit; }

		// Border sizes for NineSlice fitting (left, top, right, bottom in pixels).
		// Only used when imageFit == FImageFit::NineSlice.
		const FMargin& GetSliceMargins() const { return sliceMargins; }
		void SetSliceMargins(const FMargin& margins) { this->sliceMargins = margins; }

		// Explicit pixel size of the image within the widget rect. Vec2(-1,-1) = auto (driven by imageFit).
		const Vec2& GetBrushSize() const { return brushSize; }
		void SetBrushSize(Vec2 brushSize) { this->brushSize = brushSize; }

		// Normalized anchor point for image placement within the widget rect.
		// (0,0) = top-left, (0.5,0.5) = centered, (1,1) = bottom-right. Equivalent to CSS background-position.
		const Vec2& GetBrushPosition() const { return brushPos; }
		void SetBrushPosition(Vec2 brushPos) { this->brushPos = brushPos; }

		bool operator==(const FBrush& rhs) const;

		bool operator!=(const FBrush& rhs) const
		{
			return !operator==(rhs);
		}

	private:

		FIELD()
		FGradient gradient;

		FIELD()
		Color color;

		FIELD()
		Name imageName;

		// Explicit size of the image. Vec2(-1,-1) = auto.
		FIELD()
		Vec2 brushSize = Vec2(-1, -1);

		// Normalized anchor point (0=start, 1=end per axis). Default: centered (0.5, 0.5).
		FIELD()
		Vec2 brushPos = Vec2(0.5f, 0.5f);

		FIELD()
		FBrushTiling tiling = FBrushTiling::None;

		FIELD()
		FBrushStyle brushStyle = FBrushStyle::None;

		FIELD()
		FImageFit imageFit = FImageFit::Fill;

		// NineSlice border sizes in pixels (left, top, right, bottom). Only used when imageFit == NineSlice.
		FIELD()
		FMargin sliceMargins;

	};

}

#include "FBrush.rtti.h"
