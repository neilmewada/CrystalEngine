#pragma once

namespace CE::Metal
{

    class ComputePipeline : public Pipeline
    {
    public:
        
        ComputePipeline(Device* device, const RHI::ComputePipelineDescriptor& desc);
        virtual ~ComputePipeline();
        
        id<MTLComputePipelineState> GetMtlPipeline() const { return mtlPipeline; }
        
    private:
        
        // - ObjC -
        
        id<MTLComputePipelineState> mtlPipeline = nil;
    };
    
} // namespace CE::Metal
