#pragma once
#include "Allocator.h"

namespace CE::RHI
{
    enum class FreeListStrategy
    {
	    FirstFit = 0
    };
    
    class CORERHI_API FreeListAllocator : public VirtualAllocator
    {
    public:

        static constexpr SIZE_T kMaxAllocations = 4096;

        struct Descriptor : VirtualAllocator::Descriptor
        {
            FreeListStrategy type = FreeListStrategy::FirstFit;
        };

        ~FreeListAllocator() override;

        void Init(const Descriptor& desc);
        void Shutdown();

        VirtualAddress Allocate(SIZE_T byteCount, SIZE_T byteAlignment = 1) override;

        void DeAllocate(VirtualAddress alignedAddress) override;

        const VirtualAllocator::Descriptor& GetDescriptor() override { return descriptor; }

        SIZE_T GetNumAllocations() override
        {
            return numAllocations;
        }

        SIZE_T GetMaxNumAllocations() override
        {
            return kMaxAllocations;
        }

        FreeListStrategy GetType() const { return descriptor.type; }

    private:

        VirtualAddress AllocateFirstFit(SIZE_T byteCount, SIZE_T byteAlignment);

        struct FreeRange
        {
            VirtualAddress address = 0;
            u64 size = 0;
            FreeRange* prevFree = nullptr;
            FreeRange* nextFree = nullptr;

            VirtualAddress GetEndAddress() const { return address.ptr + size; }

            bool ContainsAddress(VirtualAddress addr) const
            {
                return address.ptr <= addr.ptr && addr.ptr < GetEndAddress().ptr;
            }
        };

        struct AllocationRecord
        {
            VirtualAddress address = 0;
            VirtualAddress alignedAddress = 0;
            SIZE_T size = 0;
        };

        FreeRange* AcquireNode();
        void ReleaseNode(FreeRange* node);

        void TrackAllocation(const AllocationRecord& record);
        bool RemoveAllocation(VirtualAddress alignedAddress, AllocationRecord& outRecord);

        Array<FreeRange*> pooledNodes;

        AllocationRecord allocationRecords[kMaxAllocations] = {};
        SIZE_T numAllocations = 0;

        Descriptor descriptor{};

        FreeRange* headNode = nullptr;

    };
    
}
