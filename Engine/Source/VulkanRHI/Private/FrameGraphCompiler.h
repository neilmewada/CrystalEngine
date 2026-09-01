#pragma once

namespace CE::Vulkan
{
	class Scope;
	class CommandList;

	class VULKANRHI_API FrameGraphCompiler final : public RHI::FrameGraphCompiler
	{
	public:

		FrameGraphCompiler(Device* device);
		virtual ~FrameGraphCompiler();

		void CompileScopesInternal(const RHI::FrameGraphCompileRequest& compileRequest) override;
		
		void CompileInternal(const RHI::FrameGraphCompileRequest& compileRequest) override;

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

		struct BarrierBatch
		{
			VkDependencyFlags dependencyFlags = 0;

			List<VkMemoryBarrier2> memoryBarriers{};
			List<VkBufferMemoryBarrier2> bufferBarriers{};
			List<VkImageMemoryBarrier2> imageBarriers{};

			List<ImageLayoutTransition> imageLayoutTransitions{};
			List<BufferFamilyTransition> bufferFamilyTransitions{};
		};

		struct ExecutionStep
		{
			Vulkan::Scope* scope = nullptr;
			BarrierBatch preBarriers;
			BarrierBatch postBarriers;
		};

		struct Submission
		{
			Vulkan::CommandQueue* queue = nullptr;
			Array<ExecutionStep> steps;
		};

		struct ExecutionPlan
		{
			Array<Submission> submissions{};
		};

	private:
        
		void DestroySyncObjects();

		void CompileCrossQueueDependencies(const RHI::FrameGraphCompileRequest& compileRequest);

		void CompileCrossQueueDependenciesInternal(const RHI::FrameGraphCompileRequest& compileRequest,
			Vulkan::Scope* current, HashSet<RHI::ScopeId>& visitedScopes);

		void CompileBarriers(const RHI::FrameGraphCompileRequest& compileRequest);

		void CompileBarriers(HashSet<RHI::ScopeId>& visitedScopes, const RHI::FrameGraphCompileRequest& compileRequest, Vulkan::Scope* current);

		Device* device = nullptr;

		// Keep track of current family index of each attachment
		HashMap<RHI::AttachmentID, u32> familyIndexByAttachment{};

		u32 numFramesInFlight = 0;

		ExecutionPlan executionPlan;

		friend class FrameGraphExecuter;
	};

} // namespace CE::Vulkan
