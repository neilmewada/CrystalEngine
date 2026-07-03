#pragma once

namespace CE::RHI
{
    struct AliasedHeapDescriptor
    {
        Name debugName = "Aliased Heap";
        u64 allocationSize = 0;
        MemoryHeapUsageFlags usageFlags = MemoryHeapUsageFlags::All;
    };
    
    class CORERHI_API AliasedHeap : public RHIResource, public IntrusiveBase
    {
    public:

        AliasedHeap(const AliasedHeapDescriptor& desc);

        ~AliasedHeap() override;

        u64 GetHeapSize() const { return heapSize; }

        MemoryHeapUsageFlags GetUsageFlags() const { return usageFlags; }

        VirtualAddress Allocate(SIZE_T byteCount, SIZE_T byteAlignment = 1);
        void DeAllocate(VirtualAddress address);

        virtual RHI::MemoryHeap* GetAllocation() = 0;

    protected:

        AliasedHeapDescriptor descriptor{};

        FreeListAllocator firstFitAllocator;

        Name debugName = "MemoryHeap";

        u64 heapSize = 0;

        MemoryHeapUsageFlags usageFlags{};

    };

} // namespace CE::RHI
