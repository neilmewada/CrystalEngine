#include "VulkanRHIPrivate.h"


namespace CE::Vulkan
{

	RenderPassCache::RenderPassCache(Device* device) : device(device)
    {

    }

    RenderPassCache::~RenderPassCache()
    {
		LockGuard<SharedMutex> lock{ mutex };

		for (auto [hash, renderPass] : renderPassCache)
		{
			delete renderPass;
		}
		renderPassCache.Clear();
    }

	VulkanRenderPass* RenderPassCache::FindOrCreate(const VulkanRenderPass::Descriptor& desc)
	{
		LockGuard<SharedMutex> lock{ mutex };

		SIZE_T hash = desc.GetHash();
		if (hash == 0)
			return nullptr;
		if (renderPassCache[hash] != nullptr)
			return renderPassCache[hash];

		VulkanRenderPass* renderPass = new VulkanRenderPass(device, desc);
		renderPass->hash = hash;
		renderPassCache[hash] = renderPass;
		return renderPass;
	}

} // namespace CE::Vulkan
