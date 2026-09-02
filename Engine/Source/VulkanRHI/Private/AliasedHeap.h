#pragma once

namespace CE::Vulkan
{

    class VULKANRHI_API AliasedHeap : public RHI::AliasedHeap
    {
    public:

        AliasedHeap(Device* device, const AliasedHeapDescriptor& desc);
        ~AliasedHeap() override;

        RHI::MemoryHeap* GetAllocation() override { return allocation; }

        bool IsCompatible(MemoryTypeMask resourceMask) override
        {
            return (resourceMask.value & (1u << allocatedMemoryTypeIndex)) != 0;
        }

    private:

        //VkDeviceMemory allocation{};
        Vulkan::MemoryHeap* allocation = nullptr;

        Device* device = nullptr;
        VkMemoryPropertyFlags memoryPropertyFlags{};
        u32 supportedMemoryTypeBitMask = 0;
        u32 allocatedMemoryTypeIndex = 0;

    };
    
} // namespace CE::Vulkan
