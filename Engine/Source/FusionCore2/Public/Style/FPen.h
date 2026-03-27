#pragma once

namespace CE
{
	ENUM()
	enum class FPenStyle : u32
	{
		None,
		Solid,
		Dashed, // Uses dashLength and dashGap
		Dotted  // Uses dashLength and dashGap (typically dashLength == thickness for round dots)
	};
	ENUM_CLASS(FPenStyle);

	STRUCT()
	struct FUSIONCORE_API FPen final
	{
		CE_STRUCT(FPen)
	public:

		FPen();
		~FPen();

		FPen(const Color& color, f32 thickness = 1.0f, FPenStyle style = FPenStyle::Solid);

		// Gradient pen. tintColor is multiplied with the gradient output (White = no tint).
		FPen(const FGradient& gradient, f32 thickness = 1.0f, const Color& tintColor = Colors::White);

		const Color& GetColor() const { return color; }
		void SetColor(const Color& penColor) { this->color = penColor; }

		const FGradient& GetGradient() const { return gradient; }
		void SetGradient(const FGradient& g) { this->gradient = g; }

		bool HasGradient() const { return gradient.IsValid(); }

		f32 GetThickness() const { return thickness; }
		void SetThickness(f32 thickness) { this->thickness = thickness; }

		FPenStyle GetStyle() const { return style; }
		void SetStyle(FPenStyle penStyle) { this->style = penStyle; }

		// Length of each dash segment in pixels. Only used when style is Dashed or Dotted.
		f32 GetDashLength() const { return dashLength; }
		void SetDashLength(f32 dashLength) { this->dashLength = dashLength; }

		// Gap between dash segments in pixels. Only used when style is Dashed or Dotted.
		f32 GetDashGap() const { return dashGap; }
		void SetDashGap(f32 dashGap) { this->dashGap = dashGap; }

		bool IsValidPen() const
		{
			return (HasGradient() || color.a > 0.001f) && thickness > 0.01f;
		}

	private:

		FIELD()
		FGradient gradient;

		FIELD()
		Color color;

		FIELD()
		f32 thickness = 0.0f;

		// Length of each dash in pixels. Only relevant for Dashed/Dotted styles.
		FIELD()
		f32 dashLength = 5.0f;

		// Gap between dashes in pixels. Only relevant for Dashed/Dotted styles.
		FIELD()
		f32 dashGap = 5.0f;

		FIELD()
		FPenStyle style = FPenStyle::None;
	};
	
}

#include "FPen.rtti.h"