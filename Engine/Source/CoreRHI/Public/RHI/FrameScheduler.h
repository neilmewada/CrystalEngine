#pragma once

namespace CE::RHI
{
	class FrameGraphCompiler;
	class FrameGraphExecuter;
    struct IScopeProducer;
	
	struct FrameSchedulerDescriptor
	{
		//! @brief Number of frames being rendered simultaneously (ex: triple buffering = 3).
		u32 numFramesInFlight = 2;
	};

	//! FrameScheduler provides user facing API to construct, compile and execute FrameGraph.
	//! Also manages transient memory allocation and aliasing.
	class CORERHI_API FrameScheduler final : public FrameGraphBuilder
	{
		CE_NO_COPY(FrameScheduler)
	private:

		FrameScheduler(const FrameSchedulerDescriptor& descriptor);

	public:
		static FrameScheduler* Create(const FrameSchedulerDescriptor& descriptor);

		virtual ~FrameScheduler();
		
		inline FrameAttachmentDatabase& GetAttachmentDatabase() const { return frameGraph->attachmentDatabase; }

		inline FrameGraph* GetFrameGraph() const { return frameGraph; }

		u32 GetFramesInFlight() const { return numFramesInFlight; }

		void ResetFramesInFlight();

		void BeginFrameGraph();

		//! @brief Compile the transient attachments, and everything.
		void Compile();

		u32 BeginExecution();

		void EndExecution();

		void SetScopeDrawList(const ScopeId& scopeId, DrawList* drawList);

		RHI::Scope* FindScope(const ScopeId& scopeId);
        
        FrameAttachment* GetFrameAttachment(AttachmentID id) const;

		inline TransientMemoryPool* GetTransientPool() const { return transientMemoryPool; }

		void WaitUntilIdle();

		static FrameScheduler* Get();

		void SetFrameGraphVariable(const Name& variableName, const RHI::FrameGraphVariable& value);
		void SetFrameGraphVariable(int imageIndex, const Name& variableName, const RHI::FrameGraphVariable& value);

		void AddScopeProducer(IScopeProducer* scopeProducer);

		const auto& GetScopeProducers() const { return scopeProducers; }

		IScopeProducer* FindScopeProducer(const Name& passName);

	private:

		u32 numFramesInFlight = 2;

		RHI::Scope* drawListScope = nullptr;

		Array<IScopeProducer*> scopeProducers{};
        
        TransientMemoryPool* transientMemoryPool = nullptr;

		FrameGraphCompiler* compiler = nullptr;
		FrameGraphExecuter* executer = nullptr;
	};
    
} // namespace CE::RHI
