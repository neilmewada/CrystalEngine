#pragma once

namespace CE::RHI
{
    //! @brief A fence is a CPU-GPU and GPU-GPU synchronization primitive that is based on a monotonically increasing uint64_t value.
    class CORERHI_API Fence : public RHIResource
    {
    protected:
        Fence(uint64_t initialValue) : RHIResource(RHI::ResourceType::Fence)
        {
            nextValue.store(initialValue, std::memory_order_relaxed);
            completedValue.store(initialValue, std::memory_order_relaxed);
        }

    public:

        virtual ~Fence() {}
        
        uint64_t NextSignalValue() { return ++nextValue; }

        uint64_t GetCompletedValue() const { return completedValue.load(std::memory_order_acquire); }
        
        virtual void RefreshCompletedValue() = 0;
        
        virtual uint64_t FetchCompletedValue()
        {
            RefreshCompletedValue();
            return GetCompletedValue();
        }

        // CPU wait until CompletedValue() >= value.
        // timeout_ns: 0 = poll; ~0ull = infinite.
        virtual bool WaitCPU(uint64_t value, uint64_t timeoutNs = ~0ull) = 0;

    protected:
        
        void SetCompletedValue(uint64_t value)
        {
            completedValue.store(value, std::memory_order_release);
        }

        std::atomic<uint64_t> nextValue{0};
        std::atomic<uint64_t> completedValue{0};
    };
    
} // namespace CE::RHI
