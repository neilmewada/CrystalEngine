#pragma once

namespace CE::Metal
{

    class RenderPassFrameBuffer : public RHI::RenderPassFrameBuffer
    {
    public:
        RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc);
        virtual ~RenderPassFrameBuffer();
        
        u32 GetWidth() override;
        u32 GetHeight() override;
        
    private:
        
        Device* device = nullptr;
        
        u32 width = 0;
        u32 height = 0;
    };
    
} // namespace CE::Metal
