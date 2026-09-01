#pragma once

namespace CE::Vulkan
{

	class VULKANRHI_API Scope : public RHI::Scope
	{
	public:
		using Super = RHI::Scope;
		using Self = Scope;

		struct ImageLayoutTransition
		{
			Vulkan::Texture* image{};
			VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
			u32 queueFamilyIndex = 0;
		};

		struct BufferFamilyTransition
		{
			Vulkan::Buffer* buffer = nullptr;
			u32 queueFamilyIndex = 0;
		};

		struct Barrier
		{
			VkPipelineStageFlags srcStageMask{};
			VkPipelineStageFlags dstStageMask{};
			bool supportsRegionalDependency = false;

			List<VkMemoryBarrier> memoryBarriers{};
			List<VkBufferMemoryBarrier> bufferBarriers{};
			List<VkImageMemoryBarrier> imageBarriers{};

			//! Just to keep track of image layout
			List<ImageLayoutTransition> imageLayoutTransitions{};
			List<BufferFamilyTransition> bufferFamilyTransitions{};
		};

		Scope(Device* device, const RHI::ScopeDescriptor& desc);
		virtual ~Scope();

		virtual bool CompileInternal(const RHI::FrameGraphCompileRequest& compileRequest) override;

		VulkanRenderPass* GetVulkanRenderPass() const { return renderPass; }

	private:

		void DestroySyncObjects();
		
		//FixedArray<VkSemaphore, RHI::Limits::MaxSwapChainImageCount> renderFinishedSemaphores{};
		//FixedArray<HashMap<RHI::ScopeId, VkSemaphore>, RHI::Limits::MaxSwapChainImageCount> signalSemaphoresByConsumerScope{};
		//FixedArray<List<VkSemaphore>, RHI::Limits::MaxSwapChainImageCount> signalSemaphores{};

		//FixedArray<VkFence, RHI::Limits::MaxSwapChainImageCount> renderFinishedFences{};

		//FixedArray<List<VkSemaphore>, RHI::Limits::MaxSwapChainImageCount> waitSemaphores{};
		//FixedArray<FrameBuffer*, RHI::Limits::MaxSwapChainImageCount> frameBuffers{};

		//FrameBuffer* frameBuffers[RHI::Limits::MaxSwapChainImageCount][RHI::Limits::MaxSwapChainImageCount] = {};

		//List<VkPipelineStageFlags> waitSemaphoreStageFlags{};

		Array<Barrier> initialBarriers{};
		Array<Barrier> barriers{};

		Device* device = nullptr;
        CommandQueue* queue = nullptr;
		VulkanRenderPass* renderPass = nullptr;
		u32 subpassIndex = 0;
        
		friend class FrameGraphCompiler;
		friend class FrameGraphExecuter;
		friend class VulkanRenderPass;
		friend class FrameBuffer;
	};
    
} // namespace CE::Vulkan
