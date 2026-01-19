
#include "MetalRHI.h"
#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    CommandQueue::CommandQueue(Device* device, HardwareQueueClassMask queueClassMask, HardwareQueueClass queueClass) : device(device)
    {
        mtlQueue = [device->GetHandle() newCommandQueue];
        
        this->queueMask = queueClassMask;
        this->queueClass = queueClass;
    }
    
    CommandQueue::~CommandQueue()
    {
        mtlQueue = nil;
    }

    bool CommandQueue::Execute(u32 count, RHI::CommandList** commandLists, RHI::Fence* fence)
    {
        // Never use this
        return false;
    }

    bool CommandQueue::Submit(const CommandQueueSubmission& submission)
    {
        if (submission.numCommandLists == 0 || submission.commandLists == nullptr)
            return false;
        
        Metal::CommandList* firstCmdList = static_cast<Metal::CommandList*>(submission.commandLists[0]);
        Metal::CommandList* lastCmdList = static_cast<Metal::CommandList*>(submission.commandLists[submission.numCommandLists - 1]);
        
        if (!firstCmdList || !lastCmdList)
            return false;
        
        id<MTLCommandBuffer> firstCmdBuffer = firstCmdList->GetMtlCommandBuffer();
        id<MTLCommandBuffer> lastCmdBuffer = lastCmdList->GetMtlCommandBuffer();
        
        if (!firstCmdBuffer || !lastCmdBuffer)
            return false;
        
        if (submission.waitFence)
        {
            Metal::Fence* fence = static_cast<Metal::Fence*>(submission.waitFence);
            
            [firstCmdBuffer encodeWaitForEvent:fence->GetEvent() value:submission.waitFenceValue];
        }
        
        if (submission.signalFence)
        {
            Metal::Fence* fence = static_cast<Metal::Fence*>(submission.signalFence);
            
            [lastCmdBuffer encodeWaitForEvent:fence->GetEvent() value:submission.signalFenceValue];
        }
        
        for (u32 i = 0; i < submission.numCommandLists; i++)
        {
            Metal::CommandList* cmdList = static_cast<Metal::CommandList*>(submission.commandLists[i]);
            
            [cmdList->GetMtlCommandBuffer() commit];
        }
        
        return true;
    }
}
