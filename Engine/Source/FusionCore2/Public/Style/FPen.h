#pragma once

namespace CE
{
	ENUM()
	enum class FPenStyle : u32
	{
		None,
		SolidLine
	};
	ENUM_CLASS(FPenStyle);

	STRUCT()
	struct FUSIONCORE_API FPen
	{
		CE_STRUCT(FPen)
	public:

		FPen();
		virtual ~FPen();

		FPen(const Color& color, f32 thickness = 1.0f, FPenStyle style = FPenStyle::SolidLine);

		const Color& GetColor() const { return color; }
		void SetColor(const Color& penColor) { this->color = penColor; }

		f32 GetThickness() const { return thickness; }
		void SetThickness(f32 thickness) { this->thickness = thickness; }

		FPenStyle GetStyle() const { return style; }
		void SetStyle(FPenStyle penStyle) { this->style = penStyle; }

		f32 GetDashLength() const { return dashLength; }
		void SetDashLength(f32 dashLength) { this->dashLength = dashLength; }

		bool IsValidPen() const
		{
			return color.a > 0 && thickness > 0;
		}

	private:

		FIELD()
		Color color;

		FIELD()
		f32 thickness = 0.0f;

		FIELD()
		f32 dashLength = 5.0f;

		FIELD()
		FPenStyle style = FPenStyle::None;
	};
	
}

#include "FPen.rtti.h"