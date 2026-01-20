#pragma once

namespace CE::Vulkan
{
    
    class Fence : public RHI::Fence
    {
    public:
        Fence(VulkanDevice* device, uint64_t initialValue);

        virtual ~Fence();

        inline VkFence GetHandle() const { return fence; }

        void RefreshCompletedValue() override
        {
            
        }
        
        bool WaitCPU(uint64_t value, uint64_t timeoutNs = ~0ull) override
        {
            
        }
        
    protected:

        VulkanDevice* device = nullptr;

        VkFence fence = nullptr;

    };

} // namespace CE::Vulkan
