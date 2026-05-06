#pragma once

namespace CE::RHI
{
    
    class CORERHI_API VirtualAllocator
    {
        CE_NO_COPY_MOVE(VirtualAllocator);
    public:

        VirtualAllocator() = default;
        virtual ~VirtualAllocator() = default;

        struct Descriptor
        {
            VirtualAddress baseAddress = 0;
            SIZE_T capacityInBytes = 0;
        };

        virtual VirtualAddress Allocate(SIZE_T byteCount, SIZE_T byteAlignment = 1) = 0;

        virtual void DeAllocate(VirtualAddress address) = 0;

        virtual const Descriptor& GetDescriptor() = 0;

        virtual SIZE_T GetNumAllocations() { return 0; }

        virtual SIZE_T GetMaxNumAllocations() { return NumericLimits<SIZE_T>::Max(); }

        VirtualAddress GetBaseAddress() { return GetDescriptor().baseAddress; }

        SIZE_T GetCapacityInBytes() { return GetDescriptor().capacityInBytes; }

    protected:

    };

} // namespace CE::RHI
