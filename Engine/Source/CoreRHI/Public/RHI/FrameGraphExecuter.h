#pragma once

namespace CE::RHI
{
	class FrameGraph;
	struct FrameGraphCompileRequest;
	class FrameGraphCompiler;
	class FrameScheduler;
	struct FrameContext;

	struct FrameGraphExecuteRequest
	{
		FrameGraph* frameGraph = nullptr;
		FrameScheduler* scheduler = nullptr;
		FrameGraphCompiler* compiler = nullptr;
	};
    
	class CORERHI_API FrameGraphExecuter
	{
		CE_NO_COPY(FrameGraphExecuter);
	public:
		virtual ~FrameGraphExecuter();

		virtual void WaitUntilIdle() = 0;

		virtual u32 GetFrameSlot() = 0;

		virtual u64 GetFrameNumber() = 0;

		virtual FrameContext WaitForNextFrame() = 0;

		virtual bool Execute(const FrameGraphExecuteRequest& executeRequest) = 0;

		virtual u32 BeginExecution(const FrameGraphExecuteRequest& executeRequest) = 0;

		virtual void EndExecution(const FrameGraphExecuteRequest& executeRequest) = 0;

		virtual void ResetFramesInFlight() = 0;

	protected:
		FrameGraphExecuter() = default;

	private:

	};

} // namespace CE::RHI
