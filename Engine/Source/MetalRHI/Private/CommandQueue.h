#pragma once

namespace CE::Metal
{

    class CommandQueue : public RHI::CommandQueue
    {
    public:
        CommandQueue(Device* device, HardwareQueueClassMask queueClassMask, HardwareQueueClass queueClass);
        ~CommandQueue();
        
        bool Execute(u32 count, RHI::CommandList** commandLists, RHI::Fence* fence = nullptr) override;
        
        id<MTLCommandQueue> GetMtlQueue() const { return mtlQueue; }
        
        bool Submit(const CommandQueueSubmission& submission) override;
        
    private:
        
        Device* device = nullptr;
        id<MTLCommandQueue> mtlQueue = nil;
        
        friend class MetalRHI;
    };
    
} // namespace CE::Metal
