#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	RenderPassFrameBuffer::RenderPassFrameBuffer(Device* device, const RHI::RenderPassFrameBufferDescriptor& desc) : RHI::RenderPassFrameBuffer(desc), device(device)
	{
		Init();
	}

	RenderPassFrameBuffer::~RenderPassFrameBuffer()
	{
		Destroy();
	}

	void RenderPassFrameBuffer::RebuildIfNeeded()
	{
		if (swapChainId == 0)
			return;

		for (int i = 0; i < desc.attachments.GetSize(); i++)
		{
			if (!desc.attachments[i].IsValid())
				continue;

			const auto& frameAttachment = desc.attachments[i];

			if (frameAttachment.GetSwapChain() != nullptr)
			{
				SwapChain* swapChain = (SwapChain*)frameAttachment.GetSwapChain();

				if (swapChain->GetSwapChainId() != swapChainId)
				{
					Destroy();
					Init();

					return;
				}
			}
		}
	}

	void RenderPassFrameBuffer::Init()
	{
		swapChainId = 0;

		for (u32 imageIndex = 0; imageIndex < framebuffers.GetSize(); imageIndex++)
		{
			FixedArray<VkImageView, RHI::Limits::Pipeline::MaxRenderAttachmentCount> attachments{};

			width = 0;
			height = 0;

			const auto& rpLayout = desc.renderPass->GetRenderPassLayout();

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
					Vulkan::Texture* image = swapChain->GetImage(imageIndex);

					swapChainId = swapChain->GetSwapChainId();

					u32 imageWidth = image->GetWidth();
					u32 imageHeight = image->GetHeight();

					RHI::ScopeAttachmentUsage attachmentUsage = rpLayout.attachmentLayouts[i].attachmentUsage;

					if (swapChain->IsFrameBufferOnly() && attachmentUsage != RHI::ScopeAttachmentUsage::Color && attachmentUsage != RHI::ScopeAttachmentUsage::Resolve)
					{
						CE_LOG(Error, All, "Failed to create RenderPassFrameBuffer: A SwapChain attachment marked as FrameBufferOnly is being used as a {} attachment!", attachmentUsage);
						width = height = 0;
						return;
					}

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
				else if (frameAttachment.GetTextureView(imageIndex) != nullptr)
				{
					Vulkan::TextureView* textureView = (Vulkan::TextureView*)frameAttachment.GetTextureView(imageIndex);

					u32 imageWidth = textureView->GetWidth();
					u32 imageHeight = textureView->GetHeight();

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
				else if (frameAttachment.GetTexture(imageIndex) != nullptr)
				{
					Vulkan::Texture* image = (Vulkan::Texture*)frameAttachment.GetTexture(imageIndex);
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

			VkFramebuffer framebuffer = nullptr;
			vkCreateFramebuffer(device->GetHandle(), &framebufferCI, VULKAN_CPU_ALLOCATOR, &framebuffer);
			this->framebuffers[imageIndex] = framebuffer;
		}
	}

	void RenderPassFrameBuffer::Destroy()
	{
		for (u32 imageIndex = 0; imageIndex < framebuffers.GetSize(); imageIndex++)
		{
			if (framebuffers[imageIndex] != nullptr)
			{
				vkDestroyFramebuffer(device->GetHandle(), framebuffers[imageIndex], VULKAN_CPU_ALLOCATOR);
				framebuffers[imageIndex] = nullptr;
			}
		}
	}
} // namespace CE::Vulkan
