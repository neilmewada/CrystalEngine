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
            resource.descriptorHash = 0;
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

    void TransientAttachmentPool::ResetFrameAllocation(u64 frameNumber)
    {
        this->frameNumber = frameNumber;
        requests.Clear();
    }

    void TransientAttachmentPool::CommitFrameAllocation()
    {
        const u32 frameSlot = frameNumber % RHI::Limits::MaxSwapChainImageCount;

        const auto& pool = pools[frameSlot];
        pool->Reset();

        for (const auto& request : requests)
        {
            if (!request.id.IsValid())
                continue;
            
            auto resourceIdentifier = Pair{ request.id, request.descriptorHash };

            if (pool->availableResources.Exists(resourceIdentifier))
            {
                pool->availableResources.Remove(resourceIdentifier);
                pool->usedResources.Add(resourceIdentifier);

                pool->allResources[resourceIdentifier].lastUsedFrame = frameNumber;
            }
            else
            {
                CE_ASSERT(!pool->usedResources.Exists(resourceIdentifier), "The same resource is being allocated twice in TransientAttachmentPool.");

                VirtualAddress address{};

                if (request.resourceType == ResourceType::Texture)
                {
                    address = attachmentAllocator->AllocateTexture(request.textureDescriptor);
                }
                else if (request.resourceType == ResourceType::Buffer)
                {
                     address = attachmentAllocator->AllocateBuffer(request.bufferDescriptor);
                }

                CE_ASSERT(address.IsValid(), "Failed to allocate Buffer/Texture!");

                pool->allResources[resourceIdentifier] = PooledAttachment{
                    .id = request.id,
                    .resourceType = request.resourceType,
                    .textureDescriptor = request.textureDescriptor,
                    .bufferDescriptor = request.bufferDescriptor,
                    .descriptorHash = request.descriptorHash,
                    .memoryOffset = address,
                    .lastUsedFrame = frameNumber
                };
            }
        }

        for (const auto& [attachmentID, resourceHash] : pool->availableResources)
        {
            auto resourceIdentifier = Pair{ attachmentID, resourceHash };

            if (frameNumber - pool->allResources[resourceIdentifier].lastUsedFrame > RHI::Limits::MaxSwapChainImageCount)
            {
	            // TODO: Free up this resource
            }
        }
    }

    void TransientAttachmentPool::AllocateBuffer(AttachmentID id, const RHI::BufferDescriptor& descriptor)
    {
        requests.Add(ResourceRequest{
            .id = id,
            .resourceType = ResourceType::Buffer,
            .bufferDescriptor = descriptor,
            .textureDescriptor = {},
            .descriptorHash = descriptor.GetHash()
        });
    }

    void TransientAttachmentPool::AllocateTexture(AttachmentID id, const RHI::TextureDescriptor& descriptor)
    {
        requests.Add(ResourceRequest{
            .id = id,
            .resourceType = ResourceType::Texture,
            .bufferDescriptor = {},
            .textureDescriptor = descriptor,
            .descriptorHash = descriptor.GetHash()
        });
    }
} // namespace CE::RHI
