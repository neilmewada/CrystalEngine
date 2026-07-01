#pragma once

namespace CE::RHI
{

    struct HeapAllocationParameters
    {
        static constexpr u64 kDefaultPageSize = 512_MB;

        u64 pageSize = kDefaultPageSize;
    };

    // The allocated attachments are not aliased for now.
    class CORERHI_API AliasedAttachmentAllocator final
    {
    public:

        struct Descriptor
        {
            HeapAllocationParameters params{};
        };

        AliasedAttachmentAllocator(const Descriptor& desc);
        virtual ~AliasedAttachmentAllocator();

        VirtualAddress AllocateBuffer(const RHI::BufferDescriptor& bufferDesc);
        VirtualAddress AllocateTexture(const RHI::TextureDescriptor& textureDesc);

    private:

        struct AllocationInfo
        {
            VirtualAddress address{};
            ResourceType resourceType{};
        };

        Ptr<AliasedHeap> AddHeapPage();

        HeapAllocationParameters parameters{};

        Array<Ptr<AliasedHeap>> pages{};
    };
    
} // namespace CE::RHI
