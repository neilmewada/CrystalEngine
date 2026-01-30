#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    Fence::Fence(Device* device, uint64_t initialValue) : RHI::Fence(initialValue), device(device)
    {
        event = [device->GetHandle() newSharedEvent];
        
        event.signaledValue = initialValue;
    }

    Fence::~Fence() noexcept
    {
        
    }

    void Fence::RefreshCompletedValue()
    {
        SetCompletedValue(event.signaledValue);
    }

    bool Fence::WaitCPU(uint64_t value, uint64_t timeoutNs)
    {
        if (event.signaledValue >= value)
        {
            RefreshCompletedValue();
            return true;
        }
        
        MTLSharedEventListener* listener = [[MTLSharedEventListener alloc] init];
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        
        [event notifyListener:listener
                      atValue:value
                        block:^(__unused id<MTLSharedEvent> e, __unused uint64_t v) {
                            dispatch_semaphore_signal(semaphore);
                        }
        ];
        
        dispatch_time_t timeout = (timeoutNs == ~0ull)
                    ? DISPATCH_TIME_FOREVER
                    : dispatch_time(DISPATCH_TIME_NOW, (int64_t)timeoutNs);
        
        long result = dispatch_semaphore_wait(semaphore, timeout);
        
        RefreshCompletedValue();
        
        return result == 0;
    }
    
} // namespace CE::Metal
