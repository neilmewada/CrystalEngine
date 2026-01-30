#pragma once

namespace CE::Metal
{

    class DeviceLimits : public RHI::DeviceLimits
    {
    public:
        
        DeviceLimits(Device* device);
        ~DeviceLimits();
        
    private:
        
        Device* device = nullptr;
    };
    
} // namespace CE::Metal
