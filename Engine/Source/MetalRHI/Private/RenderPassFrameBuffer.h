#pragma once

namespace CE::Metal
{

    class RenderPassFrameBuffer : public RHI::RenderPassFrameBuffer
    {
    public:
        RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc);
        virtual ~RenderPassFrameBuffer();
        
    private:
        
        Device* device = nullptr;
        
    };
    
} // namespace CE::Metal
