#pragma once

namespace CE::Vulkan
{

	class VULKANRHI_API FrameGraphExecuter final : public RHI::FrameGraphExecuter
	{
	public:
		using Super = RHI::FrameGraphExecuter;
		using Self = Vulkan::FrameGraphExecuter;

		FrameGraphExecuter(Device* device);
		~FrameGraphExecuter() override;

		u32 GetFrameSlot() override { return currentSubmissionIndex; }

		u64 GetFrameCounter() override { return frameCounter; }

		void WaitUntilIdle() override;

		u32 BeginFrame() override;
		bool Execute(const FrameGraphExecuteRequest& executeRequest) override;

		u32 BeginExecution(const RHI::FrameGraphExecuteRequest& executeRequest) override;

		void EndExecution(const RHI::FrameGraphExecuteRequest& executeRequest) override;

		void ResetFramesInFlight() override;

	private:

		bool ExecuteScope(const RHI::FrameGraphExecuteRequest& executeRequest, Vulkan::Scope* scope, HashSet<RHI::ScopeId>& executedScopes, 
			HashSet<Vulkan::SwapChain*>& usedSwapChains);

		Device* device = nullptr;
		FrameGraphCompiler* compiler = nullptr;

		u64 frameCounter = 0;
		u32 currentSubmissionIndex = 0;
	};
    
} // namespace CE::Vulkan
