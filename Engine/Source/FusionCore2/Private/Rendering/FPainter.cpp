#include "FusionCore.h"

namespace CE
{
	FPainter::FPainter(FLayer* layer) : owningLayer(layer), drawList(owningLayer->drawList)
	{
		
	}

	void FPainter::Begin()
	{
		isRecording = true;

		drawList->Clear();
	}

	void FPainter::End()
	{
		drawList->Finalize();

		isRecording = false;
	}

	void FPainter::PushCoordinateSpace(const FAffineTransform& transform)
	{
		if (coordinateSpaceStack.IsEmpty())
		{
			coordinateSpaceStack.Insert({
				.transform = transform
			});
		}
		else
		{
			coordinateSpaceStack.Insert({
				.transform = coordinateSpaceStack.Last().transform * transform
			});
		}
	}

	void FPainter::PopCoordinateSpace()
	{
		if (coordinateSpaceStack.IsEmpty())
			return;

		coordinateSpaceStack.RemoveLast();
	}

	FAffineTransform FPainter::GetCurrentTransform()
	{
		if (coordinateSpaceStack.IsEmpty())
			return FAffineTransform::Identity();

		return coordinateSpaceStack.Last().transform;
	}

} // namespace CE

