#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{

    RenderPassFrameBuffer::RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc)
        : RHI::RenderPassFrameBuffer(desc)
        , device(device)
    {
        for (int i = 0; i < desc.attachments.GetSize(); i++)
        {
            if (desc.attachments[i].GetSwapChain() != nullptr)
            {
                width = desc.attachments[i].GetSwapChain()->GetWidth();
                height = desc.attachments[i].GetSwapChain()->GetHeight();
            }
            else if (desc.attachments[i].GetTextureView(0) != nullptr)
            {
                width = desc.attachments[i].GetTextureView(0)->GetTexture()->GetWidth();
                height = desc.attachments[i].GetTextureView(0)->GetTexture()->GetHeight();
            }
            else if (desc.attachments[i].GetTexture(0) != nullptr)
            {
                width = desc.attachments[i].GetTexture(0)->GetWidth();
                height = desc.attachments[i].GetTexture(0)->GetHeight();
            }
        }
    }

    RenderPassFrameBuffer::~RenderPassFrameBuffer()
    {
        
    }

    u32 RenderPassFrameBuffer::GetWidth()
    {
        return width;
    }
    
    u32 RenderPassFrameBuffer::GetHeight()
    {
        return height;
    }
}
