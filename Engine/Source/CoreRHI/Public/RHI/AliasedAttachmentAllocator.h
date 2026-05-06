#pragma once

namespace CE::RHI
{

    struct HeapAllocationParameters
    {
        static constexpr u64 kDefaultPageSize = 256_MB;

        u64 pageSize = kDefaultPageSize;
    };

    class CORERHI_API AliasedAttachmentAllocator final
    {
    public:

        struct Descriptor
        {
            HeapAllocationParameters params{};
        };

        AliasedAttachmentAllocator(const Descriptor& desc);
        virtual ~AliasedAttachmentAllocator();

    private:

        Ptr<AliasedHeap> AddHeapPage();

        HeapAllocationParameters parameters{};

        Array<Ptr<AliasedHeap>> pages{};
    };
    
} // namespace CE::RHI
