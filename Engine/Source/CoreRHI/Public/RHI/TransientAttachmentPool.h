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
            ResourceHash hash = 0;
            VirtualAddress memoryOffset = 0;
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
                        available.Add(resource.hash);
	                }
                }
            }
        };

        TransientAttachmentPool(const HeapAllocationParameters& heapParams);
        virtual ~TransientAttachmentPool();
        
        void BeginFrame(u64 frameNumber);
        void EndFrame();

        void AllocateBuffer(AttachmentID id, const RHI::BufferDescriptor& descriptor);
        void AllocateTexture(AttachmentID id, const RHI::TextureDescriptor& descriptor);

    protected:

        struct ResourceRequest
        {
            AttachmentID id{};
            RHI::ResourceType resourceType = ResourceType::None;
            RHI::BufferDescriptor bufferDescriptor{};
            RHI::TextureDescriptor textureDescriptor{};
            ResourceHash hash = 0;
        };

        StaticArray<UniquePtr<ResourcePool>, RHI::Limits::MaxSwapChainImageCount> pools;

        UniquePtr<AliasedAttachmentAllocator> attachmentAllocator;

        Array<ResourceRequest> requests;
        
        u64 frameNumber = 0;
    };

} // namespace CE::RHI
