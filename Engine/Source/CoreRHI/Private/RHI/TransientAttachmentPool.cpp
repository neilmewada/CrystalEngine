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

} // namespace CE::RHI
