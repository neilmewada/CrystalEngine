#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{

    RenderPassFrameBuffer::RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc)
        : RHI::RenderPassFrameBuffer(desc)
        , device(device)
    {
        
    }

    RenderPassFrameBuffer::~RenderPassFrameBuffer()
    {
        
    }

}
