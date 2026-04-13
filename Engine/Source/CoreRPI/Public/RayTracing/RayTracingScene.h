#pragma once

namespace CE::RPI
{

    class CORERPI_API RayTracingScene final
    {
    public:

        RayTracingScene();

    private:

        RHI::Buffer*            sbtBuffer = nullptr;
        RHI::RayTracingTlas*    tlas = nullptr;

    };
    
} // namespace CE::RPI
