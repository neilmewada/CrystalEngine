#pragma once

namespace CE::Vulkan
{

    class VULKANRHI_API AliasedHeap : public RHI::AliasedHeap
    {
    public:

        AliasedHeap(VulkanDevice* device, const AliasedHeapDescriptor& desc);
        ~AliasedHeap() override;

    private:

        VkDeviceMemory allocation{};

        VulkanDevice* device = nullptr;
        VkMemoryPropertyFlags memoryPropertyFlags{};
        u32 supportedMemoryTypeBitMask = 0;
        u32 allocatedMemoryTypeIndex = 0;

    };
    
} // namespace CE::Vulkan
