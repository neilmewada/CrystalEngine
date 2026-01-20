#pragma once

namespace CE::Vulkan
{

	class VULKANRHI_API RenderPassCache final
	{
	public:
		RenderPassCache(Device* device);
		virtual ~RenderPassCache();

		VulkanRenderPass* FindOrCreate(const VulkanRenderPass::Descriptor& desc);

	private:
		Device* device = nullptr;

		SharedMutex mutex{};
		HashMap<SIZE_T, VulkanRenderPass*> renderPassCache{};
	};
    
} // namespace CE::Vulkan
