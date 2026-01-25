#pragma once

namespace CE::RPI
{

    CLASS()
    class CORERPI_API CopyPass : public GpuPass
    {
        CE_CLASS(CopyPass, GpuPass)
    protected:

        CopyPass();
        
    public:

        virtual ~CopyPass();

        void ProduceScopes(RHI::FrameScheduler* scheduler) override;

    protected:

    };
    
} // namespace CE

#include "CopyPass.rtti.h"
