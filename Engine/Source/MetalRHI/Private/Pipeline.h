#pragma once

namespace CE::Metal
{
    class GraphicsPipeline;
    
    class PipelineLayout : public RHI::IPipelineLayout
    {
    public:
        
        virtual ~PipelineLayout();
        
        bool IsCompatibleWith(PipelineLayout* other);
        
        bool IsCompatibleWith(IPipelineLayout* other) override
        {
            if (!other)
                return false;
            return IsCompatibleWith(static_cast<PipelineLayout*>(other));
        }
        
    protected:
        
        Device* device = nullptr;
        
        RHI::PipelineDescriptor desc{};
    };

    class Pipeline : public PipelineLayout, public RHI::RHIResource
    {
    public:
        
        Pipeline(Device* device, const RHI::PipelineDescriptor& desc);
        
        virtual ~Pipeline();
        
    protected:
        
        
    };

} // namespace CE::Metal
