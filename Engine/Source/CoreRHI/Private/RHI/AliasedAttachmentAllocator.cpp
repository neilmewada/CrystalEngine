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

	VirtualAddress AliasedAttachmentAllocator::AllocateBuffer(const RHI::BufferDescriptor& bufferDesc)
	{
		for (int i = 0; i < pages.GetSize(); i++)
		{
			VirtualAddress address = pages[i]->Allocate(bufferDesc.bufferSize, Math::Min<u64>(bufferDesc.alignment, 1));
			if (address.IsValid())
				return address;
		}

		AddHeapPage();
		return pages.GetLast()->Allocate(bufferDesc.bufferSize, Math::Min<u64>(bufferDesc.alignment, 1));
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
