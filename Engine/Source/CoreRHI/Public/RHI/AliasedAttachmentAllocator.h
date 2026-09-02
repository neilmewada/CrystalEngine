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

        struct Allocation
        {
            Ptr<AliasedHeap> page;
            VirtualAddress address{};
        };

        AliasedAttachmentAllocator(const Descriptor& desc);
        virtual ~AliasedAttachmentAllocator();

        Allocation AllocateBuffer(const RHI::BufferDescriptor& bufferDesc, RHI::Buffer** outBuffer);
        Allocation AllocateTexture(const RHI::TextureDescriptor& textureDesc, RHI::Texture** outTexture);

        void DeAllocate(Allocation allocation);

    private:

        struct AllocationInfo
        {
            VirtualAddress address{};
            ResourceType resourceType{};
        };

        Ptr<AliasedHeap> AddHeapPage(MemoryTypeMask compatibleMemoryTypes, u64 minimumSize);

        HeapAllocationParameters parameters{};

        Array<Ptr<AliasedHeap>> pages{};
    };
    
} // namespace CE::RHI
