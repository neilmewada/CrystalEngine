#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    Device::Device(MetalRHI* rhi) : rhi(rhi)
    {
        
    }

    Device::~Device()
    {
        
    }
    
    void Device::Initialize()
    {
        mtlDevice = MTLCreateSystemDefaultDevice();
        
        deviceLimits = new DeviceLimits(this);
        
        isInitialized = true;
        
        primaryQueue = new CommandQueue(this, RHI::HardwareQueueClassMask::All, RHI::HardwareQueueClass::Graphics);
        
        computeQueue = new CommandQueue(this, RHI::HardwareQueueClassMask::Compute, RHI::HardwareQueueClass::Compute);
        
        transferQueue = new CommandQueue(this, RHI::HardwareQueueClassMask::Transfer, RHI::HardwareQueueClass::Transfer);
    }

    void Device::PreShutdown()
    {
        isInitialized = false;
    }

    void Device::Shutdown()
    {
        delete primaryQueue; primaryQueue = nullptr;
        delete computeQueue; computeQueue = nullptr;
        delete transferQueue; transferQueue = nullptr;
        
        delete deviceLimits; deviceLimits = nullptr;
        
        mtlDevice = nil;
    }
}
