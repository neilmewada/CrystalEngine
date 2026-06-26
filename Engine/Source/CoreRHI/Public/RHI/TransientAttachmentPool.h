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
            VirtualAddress memoryOffset = 0;
            u64 lastUsedFrame = 0;
        };

        struct ResourcePool
        {
            HashMap<Pair<AttachmentID, ResourceHash>, PooledAttachment> allResources;

            HashSet<Pair<AttachmentID, ResourceHash>> availableResources;
            HashSet<Pair<AttachmentID, ResourceHash>> usedResources;

            ~ResourcePool() noexcept;

            void Reset()
            {
                availableResources.Clear();
                usedResources.Clear();

                for (const auto& [id, resource] : allResources)
                {
                    availableResources.Add({ resource.id, resource.descriptorHash });
                }
            }
        };

        TransientAttachmentPool(const HeapAllocationParameters& heapParams);
        virtual ~TransientAttachmentPool();
        
        void BeginFrameAllocation(u64 frameNumber);
        void EndFrameAllocation();

        void AllocateBuffer(AttachmentID id, const RHI::BufferDescriptor& descriptor);
        void AllocateTexture(AttachmentID id, const RHI::TextureDescriptor& descriptor);

    protected:

        struct ResourceRequest
        {
            AttachmentID id{};
            RHI::ResourceType resourceType = ResourceType::None;
            RHI::BufferDescriptor bufferDescriptor{};
            RHI::TextureDescriptor textureDescriptor{};
            ResourceHash descriptorHash = 0;
        };

        StaticArray<UniquePtr<ResourcePool>, RHI::Limits::MaxSwapChainImageCount> pools;

        UniquePtr<AliasedAttachmentAllocator> attachmentAllocator;

        Array<ResourceRequest> requests;
        
        u64 frameNumber = 0;
    };

} // namespace CE::RHI
