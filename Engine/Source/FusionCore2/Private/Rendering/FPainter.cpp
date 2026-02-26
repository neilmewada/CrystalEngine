#include "FusionCore.h"

namespace CE
{
	FPainter::FPainter(FLayer* layer) : owningLayer(layer)
	{
		
	}

	void FPainter::Begin()
	{
		isRecording = true;
	}

	void FPainter::End()
	{
		isRecording = false;
	}

} // namespace CE

