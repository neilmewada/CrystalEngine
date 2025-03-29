#pragma once
#include "Shader/ComputeShader.h"

namespace CE::RPI
{
    CLASS()
    class CORERPI_API ComputePass : public GpuPass
    {
        CE_CLASS(ComputePass, GpuPass)
    protected:

        ComputePass();
        
    public:

        virtual ~ComputePass();

        void ProduceScopes(RHI::FrameScheduler* scheduler) override;

        void SetShader(RPI::ComputeShader* shader);

        PassAttachmentSizeSource dispatchSizeSource;

    protected:

        RPI::ComputeShader* shader = nullptr;

    };
    
} // namespace CE

#include "ComputePass.rtti.h"
