#pragma once

namespace CE::RHI
{
	class FrameScheduler;

	struct FrameGraphCompileRequest
	{
		FrameGraph* frameGraph = nullptr;

		TransientAttachmentPool* transientPool = nullptr;

		bool shrinkPool = false;

		u64 frameNumber = 0;
		u32 frameSlot = 0;
		u32 numFramesInFlight = RHI::Limits::MaxFramesInFlight;
	};
    
	class CORERHI_API FrameGraphCompiler
	{
		CE_NO_COPY(FrameGraphCompiler);
	protected:
		FrameGraphCompiler() = default;

	public:

		virtual ~FrameGraphCompiler() = default;

		void Compile(const FrameGraphCompileRequest& compileRequest);

	protected:

		void CompileScopes(const FrameGraphCompileRequest& compileRequest);

		void CompileTransientAttachments(const FrameGraphCompileRequest& compileRequest);

		virtual void CompileScopesInternal(const FrameGraphCompileRequest& compileRequest) = 0;

		virtual void CompileInternal(const FrameGraphCompileRequest& compileRequest) = 0;

	};

} // namespace CE::RHI
