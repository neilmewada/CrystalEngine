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

	Ptr<FrameBuffer> FrameBufferCache::FindOrCreate(Device* device, Scope* scope, u32 frameSlot, u32 imageIndex)
	{
		Key key{};
		key.layers = 1;
		key.renderPassHash = scope->GetVulkanRenderPass()->GetHash();

		Vulkan::Scope* current = scope;
		HashSet<RHI::AttachmentID> addedAttachments{};

		while (current != nullptr)
		{
			for (RHI::ScopeAttachment* attachment : current->GetAttachments())
			{
				if (!attachment->IsImageAttachment())
					continue;
				auto frameAttachment = attachment->GetFrameAttachment();
				if (!frameAttachment)
					continue;
				if (addedAttachments.Exists(frameAttachment->GetId()))
					continue;

				switch (attachment->GetUsage())
				{
				case RHI::ScopeAttachmentUsage::None:
				case RHI::ScopeAttachmentUsage::Copy:
				case RHI::ScopeAttachmentUsage::Shader:
				case RHI::ScopeAttachmentUsage::COUNT:
					continue;
				}

				RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)attachment;
				RHI::RHIResource* resource = nullptr;

				if (frameAttachment->IsSwapChainAttachment())
				{
					auto swapChainAttachment = (RHI::SwapChainFrameAttachment*)frameAttachment;
					auto vulkanSwapChain = (Vulkan::SwapChain*)swapChainAttachment->GetSwapChain();

					resource = vulkanSwapChain->GetImage(imageIndex);
				}
				else
				{
					resource = frameAttachment->GetResource(frameSlot);
				}

				if (!resource)
					continue;

				Texture* image = nullptr;
				VkImageView imageView = nullptr;

				if (resource->GetResourceType() == RHI::ResourceType::Texture)
				{
					image = (Texture*)resource;
					imageView = image->GetImageView();
				}
				else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
				{
					TextureView* textureView = (TextureView*)resource;
					image = (Texture*)textureView->GetTexture();
					imageView = textureView->GetImageView();
				}
				else
				{
					continue;
				}

				//Texture* image = dynamic_cast<Texture*>(resource);
				if (!image || imageView == nullptr)
					continue;

				if (key.width == 0 || key.height == 0)
				{
					key.width = image->GetWidth();
					key.height = image->GetHeight();
				}
				else
				{
					if (key.width != image->GetWidth() || key.height != image->GetHeight())
					{
						key.width = key.height = 0;
						CE_LOG(Error, All, "Failed to create vulkan framebuffer: Width or height mismatch!");
						return nullptr;
					}
				}

				key.attachmentHandles.Add((SIZE_T)imageView);

				addedAttachments.Add(frameAttachment->GetId());
			}

			current = (Vulkan::Scope*)current->GetNextSubPass();
		}

		auto it = cachedFrameBuffers.Find(key);
		if (it != cachedFrameBuffers.end() && it->second.frameBuffer != nullptr)
		{
			return it->second.frameBuffer;
		}

		cachedFrameBuffers[key] = Entry{ new FrameBuffer(device, scope, frameSlot, imageIndex) };

		return nullptr;
	}

} // namespace CE::Vulkan
