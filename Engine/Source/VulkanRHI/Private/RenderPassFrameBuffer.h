#pragma once

namespace CE::Vulkan
{

    class VULKANRHI_API RenderPassFrameBuffer : public RHI::RenderPassFrameBuffer
    {
    public:
        RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc);
        virtual ~RenderPassFrameBuffer();

		VkFramebuffer GetHandle() const { return frameBuffer; }

		u32 GetWidth() const { return width; }
		u32 GetHeight() const { return height; }

    private:

        Device* device = nullptr;
        VkFramebuffer frameBuffer = nullptr;

        u32 width = 0;
        u32 height = 0;

        u32 imageIndex = 0;
    };
    
} // namespace CE::Vulkan
