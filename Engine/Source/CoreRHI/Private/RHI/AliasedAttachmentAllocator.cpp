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

	VirtualAddress AliasedAttachmentAllocator::AllocateBuffer(const RHI::BufferDescriptor& bufferDesc, RHI::Buffer** outBuffer)
	{
		for (int i = 0; i < pages.GetSize(); i++)
		{
			VirtualAddress address = pages[i]->Allocate(bufferDesc.bufferSize, Math::Min<u64>(bufferDesc.alignment, 1));
			if (address.IsValid())
			{
				*outBuffer = RHI::gDynamicRHI->CreateBufferAliased(bufferDesc, pages[i], address);
				return address;
			}
		}

		AddHeapPage();
		VirtualAddress address = pages.GetLast()->Allocate(bufferDesc.bufferSize, Math::Max<u64>(bufferDesc.alignment, 1));
		*outBuffer = RHI::gDynamicRHI->CreateBufferAliased(bufferDesc, pages.GetLast(), address);
		return address;
	}

	VirtualAddress AliasedAttachmentAllocator::AllocateTexture(const RHI::TextureDescriptor& textureDesc, RHI::Texture** outTexture)
	{
		ResourceMemoryRequirements req{};
		RHI::gDynamicRHI->GetTextureMemoryRequirements(textureDesc, req);

		for (int i = 0; i < pages.GetSize(); i++)
		{
			VirtualAddress address = pages[i]->Allocate(req.size, Math::Max<u64>(req.offsetAlignment, 1));
			if (address.IsValid())
			{
				*outTexture = RHI::gDynamicRHI->CreateTextureAliased(textureDesc, pages[i], address);
				return address;
			}
			if (address.IsValid())
				return address;
		}

		AddHeapPage();
		VirtualAddress address = pages.GetLast()->Allocate(req.size, Math::Max<u64>(req.offsetAlignment, 1));
		*outTexture = RHI::gDynamicRHI->CreateTextureAliased(textureDesc, pages.GetLast(), address);
		return address;
	}

	void AliasedAttachmentAllocator::DeAllocate(VirtualAddress address)
	{
		for (int i = 0; i < pages.GetSize(); i++)
		{
			pages[i]->DeAllocate(address);
		}
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
