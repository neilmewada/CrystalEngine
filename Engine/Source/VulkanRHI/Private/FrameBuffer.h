#pragma once

namespace CE::Vulkan
{

	class SwapChain;
	class RenderTarget;
	class Texture;

	class FrameBuffer : public RHI::RenderTargetBuffer
	{
		CE_NO_COPY(FrameBuffer);
		CE_NO_MOVE(FrameBuffer);
	public:

		FrameBuffer(VulkanDevice* device, Scope* scope, u32 imageIndex);
		virtual ~FrameBuffer();

		FrameBuffer(VulkanDevice* device, const Array<Vulkan::Texture*>& images, RenderPass* renderPass, u32 imageIndex = 0);

		FrameBuffer(VulkanDevice* device, const Array<Vulkan::TextureView*>& imageViews, RenderPass* renderPass, u32 imageIndex = 0);

		inline VkFramebuffer GetHandle() const { return frameBuffer; }

		inline u32 GetWidth() const { return width; }
		inline u32 GetHeight() const { return height; }

	private:

		VulkanDevice* device = nullptr;
		VkFramebuffer frameBuffer = nullptr;

		u32 width = 0;
		u32 height = 0;
		u32 imageIndex = 0;
	};
    
} // namespace CE::Vulkan
