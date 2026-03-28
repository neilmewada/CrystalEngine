#include "FusionCore.h"

namespace CE
{
	FTransitionManager& FTransitionManager::Get()
	{
		static FTransitionManager instance{};
		return instance;
	}

	void FTransitionManager::Shutdown()
	{
		transitionEntries.Clear();
	}
} // namespace CE
