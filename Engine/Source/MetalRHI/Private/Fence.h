#pragma once

namespace CE::Metal
{

    class Fence : public RHI::Fence
    {
    public:
        
        Fence(Device* device, uint64_t initialValue);
        virtual ~Fence() noexcept;
        
        void RefreshCompletedValue() override;
        
        bool WaitCPU(uint64_t value, uint64_t timeoutNs = ~0ULL) override;
        
        id<MTLSharedEvent> GetEvent() const { return event; }

    private:
        
        Device* device = nullptr;
        
        id<MTLSharedEvent> event = nil;
        
        friend class CommandList;
        friend class CommandQueue;
    
    };
    
} // namespace CE::Metal

