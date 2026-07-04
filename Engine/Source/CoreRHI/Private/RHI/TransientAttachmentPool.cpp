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
            if (!request.attachmentId.IsValid())
                continue;
            
            auto resourceIdentifier = Pair{ request.attachmentId, request.descriptorHash };

            if (pool->availableResources.Exists(resourceIdentifier)) // Reuse resource handle
            {
                pool->availableResources.Remove(resourceIdentifier);
                pool->usedResources.Add(resourceIdentifier);

                pool->allResources[resourceIdentifier].lastUsedFrame = frameNumber;
            }
            else
            {
                CE_ASSERT(!pool->usedResources.Exists(resourceIdentifier), "The same resource ID is being allocated twice in TransientAttachmentPool.");

                VirtualAddress address{};

                RHI::Buffer* buffer = nullptr;
                RHI::Texture* texture = nullptr;
                RHI::TextureView* textureView = nullptr;

                if (request.resourceType == ResourceType::Texture)
                {
                    address = attachmentAllocator->AllocateTexture(request.textureDescriptor, &texture);

                    CE_ASSERT(address.IsValid(), "Failed to allocate Texture!");

                    textureView = RHI::gDynamicRHI->CreateDefaultTextureView(texture);
                }
                else if (request.resourceType == ResourceType::Buffer)
                {
                	address = attachmentAllocator->AllocateBuffer(request.bufferDescriptor, &buffer);

                    CE_ASSERT(address.IsValid(), "Failed to allocate Buffer!");
                }
                else
                {
                    CE_ASSERT(false, "Invalid transient resource type!");
                }

                pool->allResources[resourceIdentifier] = PooledAttachment{
                    .id = request.attachmentId,
                    .resourceType = request.resourceType,
                    .buffer = buffer,
                    .texture =  texture,
                    .textureView = textureView,
                    .textureDescriptor = request.textureDescriptor,
                    .bufferDescriptor = request.bufferDescriptor,
                    .descriptorHash = request.descriptorHash,
                    .allocationAddress = address,
                    .lastUsedFrame = frameNumber
                };
            }
        }

        HashSet<ResourceID> resourcesToFree{};

        for (const auto& [attachmentID, resourceHash] : pool->availableResources)
        {
            auto resourceIdentifier = Pair{ attachmentID, resourceHash };
            
            if (frameNumber - pool->allResources[resourceIdentifier].lastUsedFrame > RHI::Limits::MaxSwapChainImageCount)
            {
                attachmentAllocator->DeAllocate(pool->allResources[resourceIdentifier].allocationAddress);

                resourcesToFree.Add(resourceIdentifier);
            }
        }

        for (const auto& resourceId : resourcesToFree)
        {
            if (!pool->allResources.KeyExists(resourceId))
                continue;

            PooledAttachment& attachment = pool->allResources[resourceId];
            
            // Deleting the buffer/texture unbinds the memory from AliasedHeap
            delete attachment.buffer; attachment.buffer = nullptr;
            delete attachment.textureView; attachment.textureView = nullptr;
            delete attachment.texture; attachment.texture = nullptr;

            pool->allResources.Remove(resourceId);
            pool->usedResources.Remove(resourceId);
            pool->availableResources.Remove(resourceId);
        }
    }

    RHI::Buffer* TransientAttachmentPool::GetAllocatedBuffer(AttachmentID attachmentId, DescriptorHash descriptorHash)
    {
        auto resourceIdentifier = Pair{ attachmentId, descriptorHash };

        const u32 frameSlot = frameNumber % RHI::Limits::MaxSwapChainImageCount;

        const auto& pool = pools[frameSlot];

        if (pool->allResources.KeyExists(resourceIdentifier))
        {
            return pool->allResources[resourceIdentifier].buffer;
        }

        return nullptr;
    }

    void TransientAttachmentPool::RequestBufferAllocation(AttachmentID id, const RHI::BufferDescriptor& descriptor)
    {
        requests.Add(ResourceRequest{
            .attachmentId = id,
            .resourceType = ResourceType::Buffer,
            .bufferDescriptor = descriptor,
            .textureDescriptor = {},
            .descriptorHash = descriptor.GetHash()
        });
    }

    void TransientAttachmentPool::RequestTextureAllocation(AttachmentID id, const RHI::TextureDescriptor& descriptor)
    {
        requests.Add(ResourceRequest{
            .attachmentId = id,
            .resourceType = ResourceType::Texture,
            .bufferDescriptor = {},
            .textureDescriptor = descriptor,
            .descriptorHash = descriptor.GetHash()
        });
    }
} // namespace CE::RHI
