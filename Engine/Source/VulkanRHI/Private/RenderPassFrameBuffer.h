#pragma once

namespace CE::Vulkan
{

    class VULKANRHI_API RenderPassFrameBuffer : public RHI::RenderPassFrameBuffer
    {
    public:
        RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc);
        virtual ~RenderPassFrameBuffer();

		VkFramebuffer GetHandle(u32 imageIndex) const { return framebuffers[imageIndex]; }

		u32 GetWidth() const { return width; }
		u32 GetHeight() const { return height; }

    private:

        Device* device = nullptr;
		StaticArray<VkFramebuffer, RHI::Limits::MaxSwapChainImageCount> framebuffers{};

        u32 width = 0;
        u32 height = 0;

        u32 imageIndex = 0;
    };
    
} // namespace CE::Vulkan
