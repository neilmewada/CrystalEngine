#include "CoreRHI.h"

namespace CE::RHI
{
	AliasedHeap::AliasedHeap(const AliasedHeapDescriptor& desc) 
		: RHIResource(ResourceType::AliasedHeap)
		, descriptor(desc)
	{
		
	}

} // namespace CE::RHI
