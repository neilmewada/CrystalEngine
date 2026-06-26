#include "CoreRHI.h"

namespace CE::RHI
{
	AliasedHeap::AliasedHeap(const AliasedHeapDescriptor& desc) 
		: RHIResource(ResourceType::AliasedHeap)
		, descriptor(desc)
	{
		
	}

	VirtualAddress AliasedHeap::Allocate(SIZE_T byteCount, SIZE_T byteAlignment)
	{
		return firstFitAllocator.Allocate(byteCount, byteAlignment);
	}

	void AliasedHeap::DeAllocate(VirtualAddress address)
	{
		firstFitAllocator.DeAllocate(address);
	}

} // namespace CE::RHI
