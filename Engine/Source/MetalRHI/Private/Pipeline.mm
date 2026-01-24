#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    PipelineLayout::~PipelineLayout()
    {
        
    }

    bool PipelineLayout::IsCompatibleWith(PipelineLayout* other)
    {
        if (!other)
            return false;
        
        return desc.GetHash() == other->desc.GetHash();
    }

    Pipeline::Pipeline(Device* device, const RHI::PipelineDescriptor& desc) : RHI::RHIResource(RHI::ResourceType::Pipeline)
    {
        this->device = device;
        this->desc = desc;
        
    }

    Pipeline::~Pipeline()
    {
        
    }
    
    
} // namespace CE::Metal
