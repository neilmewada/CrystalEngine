#pragma once

namespace CE::Vulkan
{
    class Device;

    class DeviceLimits : public RHI::DeviceLimits
    {
    public:
        DeviceLimits(Device* device);


    private:

    };
    
} // namespace CE::Vulkan
