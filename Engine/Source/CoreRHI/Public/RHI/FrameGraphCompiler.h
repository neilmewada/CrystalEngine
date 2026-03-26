#pragma once

#include <functional>

namespace CE::RHI
{
	class FrameScheduler;
	class FrameGraphCompiler;

	struct FrameGraphCompileRequest
	{
		FrameGraph* frameGraph = nullptr;

		TransientMemoryPool* transientPool = nullptr;

		//! Set automatically by FrameGraphCompiler::Compile(). Do not set manually.
		FrameGraphCompiler* compiler = nullptr;

		u32 numFramesInFlight = 1;

		bool shrinkPool = false;
	};

	class CORERHI_API FrameGraphCompiler
	{
	protected:
		FrameGraphCompiler() = default;

	public:

		virtual ~FrameGraphCompiler() = default;

		void Compile(const FrameGraphCompileRequest& compileRequest);

		//! Queue a destructor to be called once the GPU has finished using frame slot @frameSlot.
		//! Safe to call during compilation. The callback is invoked by FlushDeletionQueue().
		void EnqueueDeletion(u32 frameSlot, std::function<void()> fn);

		//! Flush all pending deletions for @frameSlot. Call this after waiting on the
		//! execution fence for that slot, guaranteeing the GPU is done with it.
		void FlushDeletionQueue(u32 frameSlot);

		//! Flush all frame slots unconditionally. Call after vkDeviceWaitIdle in teardown paths.
		void FlushAllDeletionQueues();

	protected:

		void CompileScopes(const FrameGraphCompileRequest& compileRequest);

		void CompileTransientAttachments(const FrameGraphCompileRequest& compileRequest);

		virtual void CompileScopesInternal(const FrameGraphCompileRequest& compileRequest) = 0;

		virtual void CompileInternal(const FrameGraphCompileRequest& compileRequest) = 0;

	private:

		StaticArray<Array<std::function<void()>>, Limits::MaxSwapChainImageCount> deletionQueues{};

	};

} // namespace CE::RHI
