#pragma once

namespace CE::Vulkan
{
    
    class Fence : public RHI::Fence
    {
    public:
        Fence(Device* device, uint64_t initialValue);

        virtual ~Fence();

        inline VkSemaphore GetHandle() const { return semaphore; }

        void RefreshCompletedValue() override;
        
        bool WaitCPU(uint64_t value, uint64_t timeoutNs = ~0ull) override;
        
    protected:

        Device* device = nullptr;

		VkSemaphore semaphore = nullptr;

    };

} // namespace CE::Vulkan
