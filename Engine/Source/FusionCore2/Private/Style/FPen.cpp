#include "FusionCore.h"

namespace CE
{
	FPen::FPen()
	{
		
	}

	FPen::~FPen()
	{
		
	}

	FPen::FPen(const Color& penColor, f32 thickness, FPenStyle penStyle) : color(penColor), thickness(thickness), style(penStyle)
	{

	}

	FPen::FPen(const FGradient& gradient, f32 thickness, const Color& tintColor)
		: gradient(gradient), color(tintColor), thickness(thickness), style(FPenStyle::Solid)
	{

	}
}
