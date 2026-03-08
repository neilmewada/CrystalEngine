#include "FusionCore.h"

namespace CE
{
	void FLayerTree::MarkSyncDirty()
	{
		needsSync = true;
	}

	void FLayerTree::DoSyncIfNeeded(FWidget* rootWidget)
	{
		if (!needsSync)
			return;


	}
} // namespace CE
