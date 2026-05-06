#include "CoreRHI.h"

namespace CE::RHI
{

    TransientAttachmentPool::ResourcePool::~ResourcePool() noexcept
    {
        for (auto& [hash, resource] : allResources)
        {
            delete resource.buffer; resource.buffer = nullptr;
            delete resource.texture; resource.texture = nullptr;
            delete resource.textureView; resource.textureView = nullptr;
            resource.hash = 0;
            resource.resourceType = ResourceType::None;
        }
        allResources.Clear();
    }

    TransientAttachmentPool::TransientAttachmentPool(const HeapAllocationParameters& heapParams)
    {
        for (int i = 0; i < pools.GetSize(); i++)
        {
            pools[i] = MakeUnique<ResourcePool>();
        }

        AliasedAttachmentAllocator::Descriptor descriptor = {};
        descriptor.params = heapParams;

        attachmentAllocator = MakeUnique<AliasedAttachmentAllocator>(descriptor);
    }

    TransientAttachmentPool::~TransientAttachmentPool()
    {
        
    }

    void TransientAttachmentPool::BeginFrame(u64 frameNumber)
    {
        this->frameNumber = frameNumber;
        requests.Clear();
    }

    void TransientAttachmentPool::EndFrame()
    {
        const u32 frameSlot = frameNumber % RHI::Limits::MaxSwapChainImageCount;

        pools[frameSlot]->Reset();
        ResourcePool* pool = pools[frameSlot].Get();

        for (const ResourceRequest& request : requests)
        {
	        if (!request.id.IsValid())
                continue;

            
        }
    }

    void TransientAttachmentPool::AllocateBuffer(AttachmentID id, const RHI::BufferDescriptor& descriptor)
    {
        requests.Add(ResourceRequest{
            .id = id,
            .resourceType = ResourceType::Buffer,
            .bufferDescriptor = descriptor,
            .textureDescriptor = {},
            .hash = descriptor.GetHash()
        });
    }

    void TransientAttachmentPool::AllocateTexture(AttachmentID id, const RHI::TextureDescriptor& descriptor)
    {
        requests.Add(ResourceRequest{
            .id = id,
            .resourceType = ResourceType::Texture,
            .bufferDescriptor = {},
            .textureDescriptor = descriptor,
            .hash = descriptor.GetHash()
        });
    }
} // namespace CE::RHI
