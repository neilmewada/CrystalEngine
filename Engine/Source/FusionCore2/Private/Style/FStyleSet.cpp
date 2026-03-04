#include "FusionCore.h"

namespace CE
{
	FStyleSet::FStyleSet()
	{
		
	}

	Ref<FStyle> FStyleSet::FindStyle(CE::Name name)
	{
		if (!stylesByKey.KeyExists(name))
			return nullptr;
		return stylesByKey[name];
	}
} // namespace CE
