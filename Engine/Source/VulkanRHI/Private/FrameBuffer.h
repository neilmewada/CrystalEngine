#pragma once

namespace CE::Vulkan
{

	class SwapChain;
	class RenderTarget;
	class Texture;

	class FrameBuffer : public IntrusiveBase
	{
		CE_NO_COPY_MOVE(FrameBuffer);
	public:

		FrameBuffer(Device* device, Scope* scope, u32 frameSlot, u32 imageIndex);
		virtual ~FrameBuffer();

		FrameBuffer(Device* device, const Array<VkImageView>& imageViews, VulkanRenderPass* renderPass,
			u32 width, u32 height, u32 layers = 1);

		FrameBuffer(Device* device, const Array<Vulkan::Texture*>& images, VulkanRenderPass* renderPass, u32 imageIndex = 0);

		FrameBuffer(Device* device, const Array<Vulkan::TextureView*>& imageViews, VulkanRenderPass* renderPass, u32 imageIndex = 0);

		inline VkFramebuffer GetHandle() const { return frameBuffer; }

		inline u32 GetWidth() const { return width; }
		inline u32 GetHeight() const { return height; }

	private:

		Device* device = nullptr;
		VkFramebuffer frameBuffer = nullptr;

		u32 width = 0;
		u32 height = 0;
		u32 imageIndex = 0;
		u32 frameIndex = 0;
	};
    
} // namespace CE::Vulkan
