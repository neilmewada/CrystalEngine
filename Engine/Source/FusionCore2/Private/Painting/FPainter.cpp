#include "FusionCore.h"

namespace CE
{
	FPainter::FPainter(FLayer* layer) : layer(layer)
	{
		
	}

	void FPainter::PushTransform(const FAffineTransform& transform)
	{
		transformStack.Push(GetCurrentTransform() * transform);
	}

	void FPainter::PopTransform()
	{
		transformStack.Pop();
	}

	FAffineTransform FPainter::GetCurrentTransform()
	{
		return transformStack.NotEmpty() ? transformStack.GetLast() : FAffineTransform::Identity();
	}
} // namespace CE

