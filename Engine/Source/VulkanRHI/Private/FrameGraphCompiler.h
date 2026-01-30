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

	private:
        
		void DestroySyncObjects();

		void CompileCrossQueueDependencies(const RHI::FrameGraphCompileRequest& compileRequest);

		void CompileCrossQueueDependenciesInternal(const RHI::FrameGraphCompileRequest& compileRequest,
			Vulkan::Scope* current, HashSet<RHI::ScopeId>& visitedScopes);

		void CompileBarriers(const RHI::FrameGraphCompileRequest& compileRequest);

		void CompileBarriers(int imageIndex, const RHI::FrameGraphCompileRequest& compileRequest, Vulkan::Scope* current);

		Device* device = nullptr;

		StaticArray<List<VkSemaphore>, RHI::Limits::MaxSwapChainImageCount> imageAcquiredSemaphores{};
        StaticArray<List<VkFence>, RHI::Limits::MaxSwapChainImageCount> imageAcquiredFences{};

		StaticArray<List<VkFence>, RHI::Limits::MaxSwapChainImageCount> graphExecutionFences{};

		StaticArray<HashSet<RHI::ScopeId>, RHI::Limits::MaxSwapChainImageCount> visitedScopes{};
		StaticArray<HashSet<RHI::AttachmentID>, RHI::Limits::MaxSwapChainImageCount> usedAttachments{};

		// Keep track of current family index of each attachment
		HashMap<RHI::AttachmentID, u32> familyIndexByAttachment{};

		u32 imageCount = 0;
		u32 numFramesInFlight = 0;

		friend class FrameGraphExecuter;
	};

} // namespace CE::Vulkan
