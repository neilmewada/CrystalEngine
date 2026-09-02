#include "CoreRHI.h"

namespace CE::RHI
{
	AliasedAttachmentAllocator::AliasedAttachmentAllocator(const Descriptor& desc) : parameters(desc.params)
	{

	}

	AliasedAttachmentAllocator::~AliasedAttachmentAllocator()
	{
		pages.Clear();
	}

	AliasedAttachmentAllocator::Allocation AliasedAttachmentAllocator::AllocateBuffer(const RHI::BufferDescriptor& bufferDesc, RHI::Buffer** outBuffer)
	{
		RHI::ResourceMemoryRequirements bufferRequirements{};
		RHI::gDynamicRHI->GetBufferMemoryRequirements(bufferDesc, bufferRequirements);

		for (int i = 0; i < pages.GetSize(); i++)
		{
			if (!pages[i]->IsCompatible(bufferRequirements.compatibleMemoryTypes))
				continue;

			VirtualAddress address = pages[i]->Allocate(bufferRequirements.size, Math::Max<u64>(bufferRequirements.offsetAlignment, 1));
			if (address.IsValid())
			{
				*outBuffer = RHI::gDynamicRHI->CreateBuffer(bufferDesc, {pages[i]->GetAllocation(), address});
				return { pages[i], address };
			}
		}

		AddHeapPage(bufferRequirements.compatibleMemoryTypes);
		VirtualAddress address = pages.GetLast()->Allocate(bufferRequirements.size, Math::Max<u64>(bufferRequirements.offsetAlignment, 1));
		*outBuffer = RHI::gDynamicRHI->CreateBuffer(bufferDesc, {pages.GetLast()->GetAllocation(), address});
		return { pages.GetLast(), address};
	}

	AliasedAttachmentAllocator::Allocation AliasedAttachmentAllocator::AllocateTexture(const RHI::TextureDescriptor& textureDesc, RHI::Texture** outTexture)
	{
		RHI::ResourceMemoryRequirements textureRequirements{};
		RHI::gDynamicRHI->GetTextureMemoryRequirements(textureDesc, textureRequirements);

		for (int i = 0; i < pages.GetSize(); i++)
		{
			if (!pages[i]->IsCompatible(textureRequirements.compatibleMemoryTypes))
				continue;

			VirtualAddress address = pages[i]->Allocate(textureRequirements.size, Math::Max<u64>(textureRequirements.offsetAlignment, 1));
			if (address.IsValid())
			{
				*outTexture = RHI::gDynamicRHI->CreateTexture(textureDesc, {pages[i]->GetAllocation(), address});
				return { pages[i], address };
			}
		}

		AddHeapPage(textureRequirements.compatibleMemoryTypes);
		VirtualAddress address = pages.GetLast()->Allocate(textureRequirements.size, Math::Max<u64>(textureRequirements.offsetAlignment, 1));
		*outTexture = RHI::gDynamicRHI->CreateTexture(textureDesc, {pages.GetLast()->GetAllocation(), address});
		return { pages.GetLast(), address };
	}

	void AliasedAttachmentAllocator::DeAllocate(Allocation allocation)
	{
		if (allocation.page)
		{
			allocation.page->DeAllocate(allocation.address);
		}
	}

	Ptr<AliasedHeap> AliasedAttachmentAllocator::AddHeapPage(MemoryTypeMask compatibleMemoryTypes)
	{
		RHI::AliasedHeapDescriptor heapDesc{};
		heapDesc.debugName = "Aliased Attachment Heap";
		heapDesc.usageFlags = MemoryHeapUsageFlags::All;
		heapDesc.allocationSize = parameters.pageSize;
		heapDesc.compatibleMemoryTypes = compatibleMemoryTypes;

		AliasedHeap* page = gDynamicRHI->AllocateAliasedHeap(heapDesc);
		pages.Add(page);

		return page;
	}

} // namespace CE::RHI
