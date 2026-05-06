#include "CoreRHI.h"

namespace CE::RHI
{
	AliasedAttachmentAllocator::AliasedAttachmentAllocator(const Descriptor& desc) : parameters(desc.params)
	{
		AddHeapPage();
	}

	AliasedAttachmentAllocator::~AliasedAttachmentAllocator()
	{
		pages.Clear();
	}

	Ptr<AliasedHeap> AliasedAttachmentAllocator::AddHeapPage()
	{
		RHI::AliasedHeapDescriptor heapDesc{};
		heapDesc.debugName = "Aliased Attachment Heap";
		heapDesc.usageFlags = MemoryHeapUsageFlags::All;
		heapDesc.allocationSize = parameters.pageSize;

		AliasedHeap* page = gDynamicRHI->AllocateAliasedHeap(heapDesc);
		pages.Add(page);

		return page;
	}

} // namespace CE::RHI
