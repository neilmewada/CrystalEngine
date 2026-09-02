#pragma once

namespace CE::RHI
{

    class CORERHI_API TransientAttachmentPool
    {
    public:

        using DescriptorHash = SIZE_T;
        using ResourceID = Pair<AttachmentID, DescriptorHash>;

        struct PooledAttachment
        {
            AttachmentID id{};
            RHI::ResourceType resourceType = ResourceType::Buffer;
            RHI::Buffer* buffer = nullptr;
            RHI::Texture* texture = nullptr;
            RHI::TextureView* textureView = nullptr;
            RHI::TextureDescriptor textureDescriptor{};
            RHI::BufferDescriptor bufferDescriptor{};
            DescriptorHash descriptorHash = 0;
            VirtualAddress allocationAddress = 0;
            Ptr<AliasedHeap> allocationPage{};
            u64 lastUsedFrame = 0;
        };

        struct ResourcePool
        {
            HashMap<ResourceID, PooledAttachment> allResources;

            HashSet<ResourceID> availableResources;
            HashSet<ResourceID> usedResources;

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
        
        void ResetFrameAllocation(u64 frameNumber);

        void RequestBufferAllocation(AttachmentID id, const RHI::BufferDescriptor& descriptor);
        void RequestTextureAllocation(AttachmentID id, const RHI::TextureDescriptor& descriptor);

        void CommitFrameAllocation();

        RHI::Buffer* GetAllocatedBuffer(AttachmentID attachmentId, DescriptorHash descriptorHash);
        RHI::TextureView* GetAllocatedTexture(AttachmentID attachmentId, DescriptorHash descriptorHash);

    protected:

        struct ResourceRequest
        {
            AttachmentID attachmentId{};
            RHI::ResourceType resourceType = ResourceType::None;
            RHI::BufferDescriptor bufferDescriptor{};
            RHI::TextureDescriptor textureDescriptor{};
            DescriptorHash descriptorHash = 0;
        };

        StaticArray<UniquePtr<ResourcePool>, RHI::Limits::MaxSwapChainImageCount> pools;

        UniquePtr<AliasedAttachmentAllocator> attachmentAllocator;

        Array<ResourceRequest> requests;
        
        u64 frameNumber = 0;
    };

} // namespace CE::RHI
