#include "CoreRHI.h"

namespace CE::RHI
{
	FrameScheduler* frameSchedulerInstance = nullptr;

	FrameScheduler::FrameScheduler(const FrameSchedulerDescriptor& descriptor)
	{
		frameSchedulerInstance = this;

		frameGraph = new FrameGraph();
        transientMemoryPool = new TransientMemoryPool();
		transientAttachmentPool = new TransientAttachmentPool(descriptor.heapAllocationParameters);
		numFramesInFlight = descriptor.numFramesInFlight;
		heapAllocationParameters = descriptor.heapAllocationParameters;

		compiler = RHI::gDynamicRHI->CreateFrameGraphCompiler();
		executer = RHI::gDynamicRHI->CreateFrameGraphExecuter();
	}

	FrameScheduler* FrameScheduler::Create(const FrameSchedulerDescriptor& descriptor)
	{
		return new FrameScheduler(descriptor);
	}

	FrameScheduler::~FrameScheduler()
	{
		if (frameSchedulerInstance == this)
		{
			frameSchedulerInstance = nullptr;
		}

		delete executer; executer = nullptr;
		delete compiler; compiler = nullptr;
		delete transientMemoryPool; transientMemoryPool = nullptr;
		delete transientAttachmentPool; transientAttachmentPool = nullptr;
		delete frameGraph; frameGraph = nullptr;
	}

	void FrameScheduler::ResetFramesInFlight()
	{
		if (executer)
		{
			executer->ResetFramesInFlight();
		}
	}

	u32 FrameScheduler::GetFrameIndex()
	{
		return executer->GetFrameIndex();
	}

	u64 FrameScheduler::GetFrameCounter()
	{
		return executer->GetFrameCounter();
	}

	void FrameScheduler::BeginFrameGraph()
	{
		ZoneScoped;

		GetAttachmentDatabase().Clear();
		scopeProducers.Clear();

		FrameGraphBuilder::BeginFrameGraph(frameGraph);
	}

    void FrameScheduler::Compile()
    {
		ZoneScoped;

		if (frameGraph->presentSwapChains.NotEmpty())
		{
            // FIXME: Fix below line
			//numFramesInFlight = frameGraph->presentSwapChains[0]->GetImageCount();
		}

        FrameGraphCompileRequest compileRequest{};
        compileRequest.frameGraph = frameGraph;
        compileRequest.transientPool = transientMemoryPool;
		compileRequest.numFramesInFlight = numFramesInFlight;
		compileRequest.shrinkPool = false;
		compileRequest.frameSlot = executer->GetFrameIndex();
		
        compiler->Compile(compileRequest);
    }

    bool FrameScheduler::BeginFrame()
    {
		return executer->BeginFrame();
    }

    u32 FrameScheduler::BeginExecution()
	{
		FrameGraphExecuteRequest executeRequest{};
		executeRequest.frameGraph = frameGraph;
		executeRequest.scheduler = this;
		executeRequest.compiler = compiler;

		return executer->BeginExecution(executeRequest);
	}

	void FrameScheduler::EndExecution()
	{
		FrameGraphExecuteRequest executeRequest{};
		executeRequest.frameGraph = frameGraph;
		executeRequest.scheduler = this;
		executeRequest.compiler = compiler;

		executer->EndExecution(executeRequest);
	}

	void FrameScheduler::SetScopeDrawList(const ScopeId& scopeId, DrawList* drawList)
	{
		if (!frameGraph->scopesById.KeyExists(scopeId))
			return;

		Scope* scope = frameGraph->scopesById[scopeId];
		if (!scope)
			return;

		scope->drawList = drawList;
	}

	RHI::Scope* FrameScheduler::FindScope(const ScopeId& scopeId)
	{
		return frameGraph->scopesById[scopeId];
	}

    FrameAttachment* FrameScheduler::GetFrameAttachment(AttachmentID id) const
    {
        return GetAttachmentDatabase().FindFrameAttachment(id);
    }

	void FrameScheduler::WaitUntilIdle()
	{
		executer->WaitUntilIdle();
	}

	FrameScheduler* FrameScheduler::Get()
	{
		return frameSchedulerInstance;
	}

	void FrameScheduler::SetFrameGraphVariable(const Name& variableName, const RHI::FrameGraphVariable& value)
	{
		frameGraph->SetVariable(variableName, value);
	}

	void FrameScheduler::SetFrameGraphVariable(int imageIndex, const Name& variableName, const RHI::FrameGraphVariable& value)
	{
		frameGraph->SetVariable(imageIndex, variableName, value);
	}

	void FrameScheduler::AddScopeProducer(IScopeProducer* scopeProducer)
	{
		scopeProducers.Add(scopeProducer);
	}

	IScopeProducer* FrameScheduler::FindScopeProducer(const Name& passName)
	{
		for (int i = 0; i < scopeProducers.GetSize(); i++)
		{
			if (scopeProducers[i]->GetPassName() == passName)
			{
				return scopeProducers[i];
			}
		}

		return nullptr;
	}

} // namespace CE::RHI
