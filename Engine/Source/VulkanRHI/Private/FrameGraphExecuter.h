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

		void ResetFramesInFlight() override;

	private:

		bool ExecuteScope(const RHI::FrameGraphExecuteRequest& executeRequest, Vulkan::Scope* scope, HashSet<RHI::ScopeId>& executedScopes, 
			HashSet<Vulkan::SwapChain*>& usedSwapChains);

		Optional<VkBufferMemoryBarrier2> ResolveBufferBarrier(const FrameGraphCompiler::BufferBarrier& bufferBarrier, u32 frameSlot);
		Optional<VkImageMemoryBarrier2> ResolveImageBarrier(const FrameGraphCompiler::ImageBarrier& imageBarrier, u32 frameSlot);

		struct FrameExecutionContext
		{
			u64 fenceCompleteValue = 0;
			u64 frameNumber = 0;
			u32 frameSlot = 0;
			Array<Vulkan::CommandList*> commandLists{};
		};

		Device* device = nullptr;
		//FrameGraphCompiler* compiler = nullptr;

		FrameBufferCache frameBufferCache{};

		Vulkan::Fence* frameCompletionFence = nullptr;
		StaticArray<FrameExecutionContext, RHI::Limits::MaxFramesInFlight> frameSlots{};

		u64 frameNumber = 0;
		u32 frameSlot = 0;
	};
    
} // namespace CE::Vulkan
