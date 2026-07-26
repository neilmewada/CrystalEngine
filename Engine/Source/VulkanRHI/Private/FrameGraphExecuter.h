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

		u32 GetFrameSlot() override { return frameSlot; }

		u64 GetFrameNumber() override { return frameNumber; }

		void WaitUntilIdle() override;

		FrameContext WaitForNextFrame() override;
		bool Execute(const FrameGraphExecuteRequest& executeRequest) override;

		u32 BeginExecution(const RHI::FrameGraphExecuteRequest& executeRequest) override;

		void EndExecution(const RHI::FrameGraphExecuteRequest& executeRequest) override;

		void ResetFramesInFlight() override;

	private:

		bool ExecuteScope(const RHI::FrameGraphExecuteRequest& executeRequest, Vulkan::Scope* scope, HashSet<RHI::ScopeId>& executedScopes, 
			HashSet<Vulkan::SwapChain*>& usedSwapChains);

		struct FrameExecutionContext
		{
			u64 fenceCompleteValue = 0;
			u64 frameNumber = 0;
			u32 frameSlot = 0;
		};

		Device* device = nullptr;
		FrameGraphCompiler* compiler = nullptr;

		Vulkan::Fence* frameCompletionFence = nullptr;
		StaticArray<FrameExecutionContext, RHI::Limits::MaxFramesInFlight> frameSlots{};

		u64 frameNumber = 0;
		u32 frameSlot = 0;
	};
    
} // namespace CE::Vulkan
