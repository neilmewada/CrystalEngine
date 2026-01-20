#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	RenderPassFrameBuffer::RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc) : RHI::RenderPassFrameBuffer(desc), device(device)
	{
		FixedArray<VkImageView, RHI::Limits::Pipeline::MaxRenderAttachmentCount> attachments{};

		width = 0;
		height = 0;

		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		for (int i = 0; i < desc.attachments.GetSize(); i++)
		{
			if (!desc.attachments[i].IsValid())
				continue;

			const auto& frameAttachment = desc.attachments[i];

			if (frameAttachment.GetSwapChain() != nullptr)
			{
				SwapChain* swapChain = (SwapChain*)frameAttachment.GetSwapChain();
				u32 imageIndex = frameAttachment.GetImageIndex();
				Vulkan::Texture* image = swapChain->GetImage(imageIndex);

				u32 imageWidth = image->GetWidth();
				u32 imageHeight = image->GetHeight();

				if (width == 0 || height == 0)
				{
					width = imageWidth;
					height = imageHeight;
				}
				else if (width != imageWidth || height != imageHeight)
				{
					CE_LOG(Error, All, "Failed to create RenderPassFrameBuffer: Width and/or height mismatch!");
					width = height = 0;
					return;
				}

				attachments.Add(image->GetImageView());
			}
			else if (frameAttachment.GetTextureView() != nullptr)
			{
				Vulkan::TextureView* textureView = (Vulkan::TextureView*)frameAttachment.GetTextureView();
				
				u32 imageWidth = textureView->GetTexture()->GetWidth();
				u32 imageHeight = textureView->GetTexture()->GetHeight();

				if (width == 0 || height == 0)
				{
					width = imageWidth;
					height = imageHeight;
				}
				else if (width != imageWidth || height != imageHeight)
				{
					CE_LOG(Error, All, "Failed to create RenderPassFrameBuffer: Width and/or height mismatch!");
					width = height = 0;
					return;
				}

				attachments.Add(textureView->GetImageView());
			}
			else if (frameAttachment.GetTexture() != nullptr)
			{
				Vulkan::Texture* image = (Vulkan::Texture*)frameAttachment.GetTexture();
				u32 imageWidth = image->GetWidth();
				u32 imageHeight = image->GetHeight();

				if (width == 0 || height == 0)
				{
					width = imageWidth;
					height = imageHeight;
				}
				else if (width != imageWidth || height != imageHeight)
				{
					CE_LOG(Error, All, "Failed to create RenderPassFrameBuffer: Width and/or height mismatch!");
					width = height = 0;
					return;
				}

				attachments.Add(image->GetImageView());
			}
		}

		framebufferCI.renderPass = ((Vulkan::RenderPass*)desc.renderPass)->GetHandle();
		framebufferCI.layers = 1;
		framebufferCI.width = width;
		framebufferCI.height = height;

		framebufferCI.attachmentCount = attachments.GetSize();
		framebufferCI.pAttachments = attachments.GetData();

		vkCreateFramebuffer(device->GetHandle(), &framebufferCI, VULKAN_CPU_ALLOCATOR, &frameBuffer);
	}

	RenderPassFrameBuffer::~RenderPassFrameBuffer()
	{
		vkDestroyFramebuffer(device->GetHandle(), frameBuffer, VULKAN_CPU_ALLOCATOR);
	}
} // namespace CE::Vulkan
