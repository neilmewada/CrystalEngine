#pragma once

namespace CE::RHI
{

    class CORERHI_API TransientAttachmentPool
    {
    public:

        using ResourceHash = SIZE_T;

        struct PooledAttachment
        {
            AttachmentID id{};
            RHI::ResourceType resourceType = ResourceType::Buffer;
            RHI::Buffer* buffer = nullptr;
            RHI::Texture* texture = nullptr;
            RHI::TextureView* textureView = nullptr;
            RHI::TextureDescriptor textureDescriptor{};
            RHI::BufferDescriptor bufferDescriptor{};
            ResourceHash descriptorHash = 0;
            u64 memoryOffset = 0;
            u64 lastUsedFrame = 0;
        };

        struct ResourcePool
        {
            HashMap<ResourceHash, PooledAttachment> allResources;

            HashSet<ResourceHash> available;
            HashSet<ResourceHash> used;

            ~ResourcePool() noexcept;

            void Reset()
            {
                available.Clear();
                used.Clear();

                for (const auto& [id, resource] : allResources)
                {
	                if (resource.resourceType != ResourceType::None)
	                {
                        available.Add(resource.descriptorHash);
	                }
                }
            }
        };

        TransientAttachmentPool(const HeapAllocationParameters& heapParams);
        virtual ~TransientAttachmentPool();
        
    protected:

        StaticArray<UniquePtr<ResourcePool>, RHI::Limits::MaxSwapChainImageCount> pools;

        UniquePtr<AliasedAttachmentAllocator> attachmentAllocator;
        
    };

} // namespace CE::RHI
