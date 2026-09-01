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

		struct FrameCompileContext
		{
			CE_NO_COPY_MOVE(FrameCompileContext)
		public:

			FrameCompileContext(VkDevice device) : device(device)
			{}

			~FrameCompileContext()
			{
			}

			VkDevice device = nullptr;
			List<VkSemaphore> freeSemaphores;
		};
        
		void DestroySyncObjects();

		void CompileCrossQueueDependencies(const RHI::FrameGraphCompileRequest& compileRequest);

		void CompileCrossQueueDependenciesInternal(const RHI::FrameGraphCompileRequest& compileRequest,
			Vulkan::Scope* current, HashSet<RHI::ScopeId>& visitedScopes);

		void CompileBarriers(const RHI::FrameGraphCompileRequest& compileRequest);

		void CompileBarriers(HashSet<RHI::ScopeId>& visitedScopes, const RHI::FrameGraphCompileRequest& compileRequest, Vulkan::Scope* current);

		Device* device = nullptr;

		StaticArray<UniquePtr<FrameCompileContext>, RHI::Limits::MaxFramesInFlight> frameCompileContexts{};

		// Keep track of current family index of each attachment
		HashMap<RHI::AttachmentID, u32> familyIndexByAttachment{};

		u32 numFramesInFlight = 0;

		friend class FrameGraphExecuter;
	};

} // namespace CE::Vulkan
