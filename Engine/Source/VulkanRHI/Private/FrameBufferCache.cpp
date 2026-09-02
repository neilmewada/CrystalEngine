#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	SIZE_T FrameBufferCache::Key::GetHash() const
	{
		SIZE_T hash = renderPassHash;
		for (SIZE_T attachmentHandle : attachmentHandles)
		{
			CombineHash(hash, attachmentHandle);
		}
		CombineHash(hash, width);
		CombineHash(hash, height);
		CombineHash(hash, layers);
		return hash;
	}

	bool FrameBufferCache::Key::operator==(const Key& rhs) const
	{
		if (renderPassHash != rhs.renderPassHash ||
			width != rhs.width ||
			height != rhs.height ||
			layers != rhs.layers ||
			attachmentHandles.GetSize() != rhs.attachmentHandles.GetSize())
		{
			return false;
		}

		for (int i = 0; i < attachmentHandles.GetSize(); ++i)
		{
			if (attachmentHandles[i] != rhs.attachmentHandles[i])
				return false;
		}

		return true;
	}

	Ptr<FrameBuffer> FrameBufferCache::FindOrCreate(Device* device, Scope* scope, u32 frameSlot)
	{
		if (device == nullptr || scope == nullptr || scope->GetVulkanRenderPass() == nullptr)
			return nullptr;

		VulkanRenderPass* renderPass = scope->GetVulkanRenderPass();
		Key key{};
		key.layers = 1;
		key.renderPassHash = renderPass->GetHash();

		List<VkImageView> imageViews{};

		for (RHI::ScopeAttachment* attachment : scope->GetAttachments())
		{
			if (!attachment->IsImageAttachment())
				continue;

			if (attachment->GetUsage() == RHI::ScopeAttachmentUsage::Shader ||
				attachment->GetUsage() == RHI::ScopeAttachmentUsage::Copy)
				continue;

			RHI::FrameAttachment* frameAttachment = attachment->GetFrameAttachment();

			if (frameAttachment == nullptr)
				return nullptr;

			RHI::RHIResource* resource = nullptr;
			if (frameAttachment->IsSwapChainAttachment())
			{
				auto swapChainAttachment = (RHI::SwapChainFrameAttachment*)frameAttachment;
				auto vulkanSwapChain = (Vulkan::SwapChain*)swapChainAttachment->GetSwapChain();
				resource = vulkanSwapChain->GetCurrentImage();
			}
			else
			{
				resource = frameAttachment->GetResource(frameSlot);
			}

			if (resource == nullptr)
				return nullptr;

			Texture* image = nullptr;
			VkImageView imageView = nullptr;
			u32 imageWidth = 0;
			u32 imageHeight = 0;

			if (resource->GetResourceType() == RHI::ResourceType::Texture)
			{
				image = (Texture*)resource;
				imageView = image->GetImageView();
				imageWidth = image->GetWidth();
				imageHeight = image->GetHeight();
			}
			else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
			{
				TextureView* textureView = (TextureView*)resource;
				image = (Texture*)textureView->GetTexture();
				imageView = textureView->GetImageView();
				imageWidth = Math::Max<u32>(image->GetWidth() / Math::Pow(2, textureView->GetBaseMipLevel()), 1);
				imageHeight = Math::Max<u32>(image->GetHeight() / Math::Pow(2, textureView->GetBaseMipLevel()), 1);
			}
			else
			{
				return nullptr;
			}

			if (image == nullptr || imageView == nullptr)
				return nullptr;

			if (key.width == 0 || key.height == 0)
			{
				key.width = imageWidth;
				key.height = imageHeight;
			}
			else if (key.width != imageWidth || key.height != imageHeight)
			{
				CE_LOG(Error, All, "Failed to create vulkan framebuffer: Width or height mismatch!");
				return nullptr;
			}

			imageViews.Add(imageView);
			key.attachmentHandles.Add((SIZE_T)imageView);
		}

		if (imageViews.IsEmpty())
			return nullptr;

		auto it = cachedFrameBuffers.Find(key);
		if (it != cachedFrameBuffers.end() && it->second.frameBuffer != nullptr)
		{
			return it->second.frameBuffer;
		}

		Ptr<FrameBuffer> frameBuffer = new FrameBuffer(device, imageViews, renderPass,
			key.width, key.height, key.layers);
		cachedFrameBuffers[key] = Entry{ frameBuffer };

		return frameBuffer;
	}

} // namespace CE::Vulkan
