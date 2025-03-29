#include "CoreRHI.h"

namespace CE::RHI
{

    void FrameGraphBuilder::BeginFrameGraph(FrameGraph* frameGraph)
    {
		ZoneScoped;

		this->frameGraph = frameGraph;
		frameGraph->Clear();

		errorMessages.Clear();
		success = true;
    }

	void FrameGraphBuilder::BeginScopeGroup(const ScopeId& groupId)
	{
		curScopeGroup = {};
		curScopeGroup.groupId = groupId;
	}

	void FrameGraphBuilder::BeginScope(const ScopeId& id)
	{
		RHI::ScopeDescriptor desc{};
		desc.id = id;
		desc.queueClass = RHI::HardwareQueueClass::Graphics;
    	desc.operation = ScopeOperation::Rasterization;
		this->currentScope = RHI::gDynamicRHI->CreateScope(desc);
	}

	void FrameGraphBuilder::BeginScope(const ScopeId& id, ScopeOperation operation, HardwareQueueClass queueClass)
	{
    	RHI::ScopeDescriptor desc{};
    	desc.id = id;
    	desc.queueClass = queueClass;
    	desc.operation = operation;
    	this->currentScope = RHI::gDynamicRHI->CreateScope(desc);
	}

	bool FrameGraphBuilder::ScopeQueueClass(HardwareQueueClass queueClass)
	{
		if (!currentScope || !frameGraph)
			return false;

		currentScope->queueClass = queueClass;
		return true;
	}

	bool FrameGraphBuilder::UseAttachment(const ImageScopeAttachmentDescriptor& descriptor, ScopeAttachmentUsage usage, ScopeAttachmentAccess access)
	{
		if (!currentScope || !frameGraph)
			return false;

		FrameAttachment* frameAttachment = GetFrameAttachmentDatabase().FindFrameAttachment(descriptor.attachmentId);
		if (!frameAttachment)
			return false;

		ScopeAttachment* scopeAttachment = currentScope->FindScopeAttachment(descriptor.attachmentId);
		if (scopeAttachment == nullptr)
		{
			scopeAttachment = currentScope->EmplaceScopeAttachment<ImageScopeAttachment>(frameAttachment, usage, access, descriptor);
		}
		else
		{
			scopeAttachment->usage = usage;
			scopeAttachment->access = access;
		}

		scopeAttachment->shaderInputName = descriptor.shaderInputName;
		
		return true;
	}

	bool FrameGraphBuilder::UseShaderResourceGroup(RHI::ShaderResourceGroup* srg)
	{
		if (!currentScope || !frameGraph || !srg)
			return false;

		currentScope->AddShaderResourceGroups(srg);
		return true;
	}

	bool FrameGraphBuilder::UsePassSrgLayout(const RHI::ShaderResourceGroupLayout& srgLayout)
    {
    	if (!currentScope || !frameGraph || srgLayout.IsEmpty())
    		return false;

    	currentScope->passSrgLayout = srgLayout;
    	return true;
    }

	bool FrameGraphBuilder::UseAttachment(const BufferScopeAttachmentDescriptor& descriptor, ScopeAttachmentUsage usage, ScopeAttachmentAccess access)
	{
		if (!currentScope || !frameGraph)
			return false;

		FrameAttachment* frameAttachment = GetFrameAttachmentDatabase().FindFrameAttachment(descriptor.attachmentId);
		if (!frameAttachment)
			return false;

		ScopeAttachment* scopeAttachment = currentScope->FindScopeAttachment(descriptor.attachmentId);
		if (scopeAttachment == nullptr)
		{
			scopeAttachment = currentScope->EmplaceScopeAttachment<BufferScopeAttachment>(frameAttachment, usage, access, descriptor);
		}
		else
		{
			scopeAttachment->usage = usage;
			scopeAttachment->access = access;
		}

		scopeAttachment->shaderInputName = descriptor.shaderInputName;

		return true;
	}

	bool FrameGraphBuilder::UsePipelines(const Array<RHI::PipelineState*>& pipelines)
	{
		if (!currentScope || !frameGraph)
			return false;

		currentScope->usePipelines.AddRange(pipelines);

		return true;
	}

	bool FrameGraphBuilder::UsePipeline(RHI::PipelineState* pipeline)
	{
		if (!currentScope || !frameGraph || !pipeline)
			return false;
		if (currentScope->usePipelines.Exists(pipeline))
			return false;

		currentScope->usePipelines.Add(pipeline);

		return true;
	}

	bool FrameGraphBuilder::ExecuteOnlyIf(const Name& variableName, FrameGraphVariableComparison comparisonOp, 
		const FrameGraphVariable& comparisonValue, 
		bool shouldClear)
	{
		if (!currentScope || !frameGraph)
			return false;

		ExecuteCondition cond{};
		cond.variableName = variableName;
		cond.comparisonOperation = comparisonOp;
		cond.comparisonValue = comparisonValue;
		cond.shouldClear = shouldClear;

		currentScope->executeConditions.Add(cond);

		return true;
	}

	bool FrameGraphBuilder::SetVariableAfterExecution(const Name& variableName, const FrameGraphVariable& value)
	{
		if (!currentScope || !frameGraph)
			return false;
		
		currentScope->setVariablesAfterExecutionPerFrame.Add(variableName, value);
		
		return true;
	}

	bool FrameGraphBuilder::SetVariableAfterExecutionForAllFrames(const Name& variableName, const FrameGraphVariable& value)
	{
		if (!currentScope || !frameGraph)
			return false;

		currentScope->setVariablesAfterExecutionAllFrames.Add(variableName, value);

		return true;
	}

	bool FrameGraphBuilder::SetVariableInitialValue(const Name& variableName, const FrameGraphVariable& value)
	{
		frameGraph->SetVariable(variableName, value);
		return true;
	}

	bool FrameGraphBuilder::SetDispatchGroupCount(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
	{
		if (!currentScope || !frameGraph)
			return false;

		currentScope->groupCountX = groupCountX;
		currentScope->groupCountY = groupCountY;
		currentScope->groupCountZ = groupCountZ;

		return true;
	}

	bool FrameGraphBuilder::PresentSwapChain(SwapChain* swapChain)
	{
		if (!currentScope || !frameGraph)
			return false;
		frameGraph->numFramesInFlight = swapChain->GetImageCount();
		frameGraph->presentSwapChains.Add(swapChain);
		frameGraph->presentingScopes.Add(currentScope);
		currentScope->presentSwapChains.Add(swapChain);
		return true;
	}

	Scope* FrameGraphBuilder::EndScope()
	{
		if (!currentScope || !frameGraph)
			return nullptr;
		if (curScopeGroup.groupId.IsValid())
			curScopeGroup.scopes.Add(currentScope);

		frameGraph->scopes.Add(currentScope);
		frameGraph->scopesById[currentScope->id] = currentScope;
		return currentScope;
	}

	void FrameGraphBuilder::EndScopeGroup()
	{
		int scopeGroupIndex = frameGraph->scopeGroups.GetSize();
		frameGraph->scopeGroups.Add(curScopeGroup);
		curScopeGroup = {};

		const Array<Scope*>& scopesList = frameGraph->scopeGroups.GetLast().scopes;

		RHI::HardwareQueueClass queueClass{};

		for (int i = 0; i < scopesList.GetSize(); i++)
		{
			Scope* cur = scopesList[i];
			Scope* prev = nullptr;
			Scope* next = nullptr;
			cur->scopeGroupIndex = scopeGroupIndex;

			if (i == 0)
			{
				queueClass = cur->queueClass;
			}
			else if (queueClass != cur->queueClass)
			{
				errorMessages.Add("All scopes inside scope group must have same HardwareQueueClass.");
				CE_LOG(Error, All, errorMessages.Top());
				success = false;
				return;
			}

			if (queueClass != HardwareQueueClass::Graphics)
			{
				errorMessages.Add("Scopes can be grouped together only if they are of Graphics queue class.");
				CE_LOG(Error, All, errorMessages.Top());
				success = false;
				return;
			}

			if (i < scopesList.GetSize() - 1)
				next = scopesList[i + 1];
			if (i > 0)
				prev = scopesList[i - 1];

			cur->prevSubPass = prev;
			cur->nextSubPass = next;

			cur->prev = prev;
			cur->next = next;
		}
	}

    bool FrameGraphBuilder::EndFrameGraph()
    {
		ZoneScoped;

		return frameGraph->Build();
    }

} // namespace CE::RHI
