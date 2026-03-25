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

		const Color& GetColor() const { return color; }
		void SetColor(const Color& penColor) { this->color = penColor; }

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
			return color.a > 0.001f && thickness > 0.01f;
		}

	private:

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