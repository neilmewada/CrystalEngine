#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    FrameGraphExecuter::FrameGraphExecuter(Device* device) : device(device)
    {
        renderingFinishedFence = gDynamicRHI->CreateFence();
    }
    
    FrameGraphExecuter::~FrameGraphExecuter()
    {
        delete renderingFinishedFence; renderingFinishedFence = nullptr;
    }
    
    void FrameGraphExecuter::WaitUntilIdle()
    {
        ZoneScoped;
        
    }
    
    u32 FrameGraphExecuter::BeginExecution(const FrameGraphExecuteRequest& executeRequest)
    {
        ZoneScoped;
        
        RHI::FrameGraph* frameGraph = executeRequest.frameGraph;
        compiler = (Metal::FrameGraphCompiler*)executeRequest.compiler;
        bool swapChainExists = frameGraph->GetSwapChainCount() > 0;
        
        const Array<RHI::Scope*>& producers = frameGraph->GetProducers();
        
        {
            ZoneNamedN(_renderingFinishedFence, "_RenderingFinishedFence", true);
            
            renderingFinishedFence->WaitCPU(renderingFinishedValues[currentSubmissionIndex]);
        }
        
        if (swapChainExists)
        {
            for (int i = 0; i < frameGraph->GetSwapChainCount(); i++)
            {
                auto swapChain = (Metal::SwapChain*)frameGraph->GetSwapChain(i);
                
                bool imageAcquired = swapChain->AcquireNextImage();
                
                if (!imageAcquired)
                {
                    return RHI::Limits::MaxSwapChainImageCount;
                }
            }
        }
        
        return currentSubmissionIndex;
    }
    
    void FrameGraphExecuter::EndExecution(const FrameGraphExecuteRequest& executeRequest)
    {
        ZoneScoped;
        
        String value = String("Index: ") + currentSubmissionIndex;
        ZoneText(value.GetCString(), value.GetLength());

        RHI::FrameGraph* frameGraph = executeRequest.frameGraph;
        
        HashSet<RHI::ScopeId> executedScopes{};
        HashSet<Metal::SwapChain*> usedSwapChains{};
        
        for (auto rhiScope : frameGraph->GetEndScopes())
        {
            ExecuteScope(executeRequest, (Metal::Scope*)rhiScope, executedScopes, usedSwapChains);
        }
        
        currentSubmissionIndex = (currentSubmissionIndex + 1) % compiler->numFramesInFlight;
    }
    
    bool FrameGraphExecuter::ExecuteScope(const RHI::FrameGraphExecuteRequest& executeRequest, Metal::Scope* scope,
                                          HashSet<RHI::ScopeId>& executedScopes,
                                          HashSet<Metal::SwapChain*>& usedSwapChains)
    {
        if (!scope)
            return false;
        
        ZoneScoped;
        
        for (auto rhiProducer : scope->GetProducers())
        {
            ExecuteScope(executeRequest, (Metal::Scope*)rhiProducer, executedScopes, usedSwapChains);
        }
        
        if (executedScopes.Exists(scope->GetId()))
            return false;
        if (scope->IsSubPass() && scope->GetPrevSubPass() != nullptr)
            return false;
        
        RHI::FrameGraph* frameGraph = executeRequest.frameGraph;
        RHI::FrameScheduler* scheduler = executeRequest.scheduler;
        auto compiler = (Metal::FrameGraphCompiler*)executeRequest.compiler;
        
        bool presentRequired = false;
        
        Array<RHI::SwapChain*> presentSwapChains = frameGraph->GetPresentSwapChains();
        
        Array<Scope*> scopeChain{};
        Scope* scopeInChain = scope;
        HashSet<Metal::SwapChain*> swapChainsUsedAsAttachmentForFirstTime{};

        while (scopeInChain != nullptr)
        {
            if (scopeInChain->swapChainsUsedByAttachments.NotEmpty())
            {
                for (auto swapChain : scopeInChain->swapChainsUsedByAttachments)
                {
                    swapChainsUsedAsAttachmentForFirstTime.Add((Metal::SwapChain*)swapChain);
                }
                //isFirstSwapChainUse = true;
            }
            scopeChain.Add(scopeInChain);
            scopeInChain = (Metal::Scope*)scopeInChain->next;
        }
        
        // We remove swapchain of scopes that have already been executed.
        // So we are left with a set of swapchains that are going to be used for the first time in current frame in flight.
        for (const auto& executedScopeId : executedScopes)
        {
            auto executedScope = (Metal::Scope*)frameGraph->scopesById[executedScopeId];
            for (auto usedSwapChain : usedSwapChains)
            {
                if (swapChainsUsedAsAttachmentForFirstTime.Exists(usedSwapChain))
                {
                    swapChainsUsedAsAttachmentForFirstTime.Remove(usedSwapChain);
                }
            }
        }
        
        for (auto swapChainUsed : swapChainsUsedAsAttachmentForFirstTime)
        {
            usedSwapChains.Add(swapChainUsed);
        }
        
        Array<RHI::Scope*> consumers{};
        
        if (scope->consumers.GetSize() > 1)
        {
            consumers = scope->consumers;
        }
        else
        {
            consumers = scopeChain.Top()->consumers;
        }

        if (scopeChain.Top()->PresentsSwapChain())
        {
            presentRequired = true;
        }
        
        Metal::CommandList* cmdList = scope->cmdLists[currentSubmissionIndex];
        
        cmdList->Begin();
        {
            cmdList->SetFrameIndex(currentSubmissionIndex);
            
            for (int scopeIndex = 0; scopeIndex < scopeChain.GetSize(); scopeIndex++)
            {
                auto currentScope = (Metal::Scope*)scopeChain[scopeIndex];
                if (currentScope == nullptr)
                    continue;
                
                executedScopes.Add(currentScope->GetId());
                
                FixedArray<RHI::AttachmentClearValue, RHI::Limits::Pipeline::MaxRenderAttachmentCount> clearValues{};
                HashSet<RHI::AttachmentID> clearedAttachments{};
                
                // Loop through all subpasses
                Metal::Scope* scopeLoop = currentScope;
                while (scopeLoop != nullptr)
                {
                    for (RHI::ScopeAttachment* scopeAttachment : scopeLoop->attachments)
                    {
                        if (!scopeAttachment->IsImageAttachment() || scopeAttachment->GetFrameAttachment() == nullptr ||
                            !scopeAttachment->GetFrameAttachment()->IsImageAttachment())
                            continue;
                        if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Shader ||
                            scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Copy)
                            continue;
                        if (clearedAttachments.Exists(scopeAttachment->GetFrameAttachment()->GetId()))
                            continue;

                        RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)scopeAttachment;
                        RHI::ImageFrameAttachment* imageFrameAttachment = (RHI::ImageFrameAttachment*)scopeAttachment->GetFrameAttachment();
                        
                        RHI::AttachmentClearValue clearValue{};
                        
                        if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::DepthStencil)
                        {
                            clearValue.clearValueDepth = scopeAttachment->GetLoadStoreAction().clearValueDepth;
                            clearValue.clearValueStencil = scopeAttachment->GetLoadStoreAction().clearValueStencil;
                        }
                        else
                        {
                            clearValue.clearValue = scopeAttachment->GetLoadStoreAction().clearValue;
                        }
                        
                        clearedAttachments.Add(scopeAttachment->GetFrameAttachment()->GetId());

                        clearValues.Add(clearValue);
                    }
                    
                    scopeLoop = (Metal::Scope*)scopeLoop->nextSubPass;
                }
                
                // Graphics operation
                if (currentScope->queueClass == RHI::HardwareQueueClass::Graphics)
                {
                    cmdList->ClearShaderResourceGroups();
                    
                    cmdList->BeginRenderPass(currentScope->renderPass, currentScope->frameBuffer, clearValues.GetData());
                    {
                        RHI::ViewportState viewport{};
                        viewport.x = viewport.y = 0;
                        viewport.width = currentScope->frameBuffer->GetWidth();
                        viewport.height = currentScope->frameBuffer->GetHeight();
                        viewport.minDepth = 0.0f;
                        viewport.maxDepth = 1.0f;
                        cmdList->SetViewports(1, &viewport);
                        
                        RHI::ScissorState scissor{};
                        scissor.x = viewport.x;
                        scissor.y = viewport.y;
                        scissor.width = viewport.width;
                        scissor.height = viewport.height;
                        cmdList->SetScissors(1, &scissor);
                        
                        while (currentScope != nullptr)
                        {
                            RHI::DrawList* drawList = currentScope->drawList;
                            
                            // Submit draw items
                            for (int i = 0; drawList != nullptr && i < drawList->GetDrawItemCount(); i++)
                            {
                                for (auto srg : currentScope->externalShaderResourceGroups)
                                {
                                    cmdList->SetShaderResourceGroup(srg);
                                }
                                
                                if (currentScope->passShaderResourceGroup)
                                    cmdList->SetShaderResourceGroup(currentScope->passShaderResourceGroup);
                                if (currentScope->subpassShaderResourceGroup)
                                    cmdList->SetShaderResourceGroup(currentScope->subpassShaderResourceGroup);
                                
                                const auto& drawItemProperties = drawList->GetDrawItem(i);
                                const RHI::DrawItem* drawItem = drawItemProperties.item;
                                
                                if (drawItem->enabled)
                                {
                                    RHI::PipelineState* pipeline = drawItem->pipelineState;
                                    if (pipeline)
                                    {
                                        cmdList->BindPipelineState(pipeline);
                                    }
                                    
                                    // Bind SRGs
                                    for (int j = 0; j < drawItem->shaderResourceGroupCount; j++)
                                    {
                                        if (drawItem->shaderResourceGroups[j] != nullptr)
                                        {
                                            cmdList->SetShaderResourceGroup(drawItem->shaderResourceGroups[j]);
                                        }
                                    }

                                    for (int j = 0; j < drawItem->uniqueShaderResourceGroupCount; j++)
                                    {
                                        if (drawItem->uniqueShaderResourceGroups[j] != nullptr)
                                        {
                                            cmdList->SetShaderResourceGroup(drawItem->uniqueShaderResourceGroups[j]);
                                        }
                                    }
                                    
                                    cmdList->CommitShaderResources();
                                    
                                    cmdList->BindVertexBuffers(0, drawItem->vertexBufferViewCount, drawItem->vertexBufferViews);
                                    
                                    if (drawItem->rootConstantSize > 0 && drawItem->rootConstants != nullptr && (int)drawItem->rootConstantSize % 4 == 0)
                                    {
                                        cmdList->SetRootConstants(0, (u32)drawItem->rootConstantSize / 4, drawItem->rootConstants);
                                    }
                                    
                                    if (drawItem->arguments.type == RHI::DrawArgumentsType::DrawArgumentsIndexed)
                                    {
                                        cmdList->BindIndexBuffer(*drawItem->indexBufferView);
                                        cmdList->DrawIndexed(drawItem->arguments.indexedArgs);
                                    }
                                    else if (drawItem->arguments.type == RHI::DrawArgumentsType::DrawArgumentsLinear)
                                    {
                                        cmdList->DrawLinear(drawItem->arguments.linearArgs);
                                    }
                                }
                            }
                            
                            if (currentScope->nextSubPass == nullptr) // No more subpasses left
                            {
                                cmdList->ClearShaderResourceGroups();
                                break;
                            }
                            else
                            {
                                // TODO: Implement subpasses
                            }
                        }
                    }
                    cmdList->EndRenderPass();
                }
                else if (currentScope->queueClass == RHI::HardwareQueueClass::Compute)
                {
                    cmdList->ClearShaderResourceGroups();

                    // TODO: Add compute pass
                    RHI::PipelineState* pipelineToUse = nullptr;

                    for (RHI::PipelineState* pipeline : currentScope->usePipelines)
                    {
                        if (pipeline != nullptr && pipeline->GetPipelineType() == RHI::PipelineStateType::Compute)
                        {
                            pipelineToUse = pipeline;
                        }
                    }

                    if (pipelineToUse != nullptr)
                    {
                        cmdList->BindPipelineState(pipelineToUse);

                        for (auto srg : currentScope->externalShaderResourceGroups)
                        {
                            cmdList->SetShaderResourceGroup(srg);
                        }

                        if (currentScope->passShaderResourceGroup)
                            cmdList->SetShaderResourceGroup(currentScope->passShaderResourceGroup);
                        if (currentScope->subpassShaderResourceGroup)
                            cmdList->SetShaderResourceGroup(currentScope->subpassShaderResourceGroup);

                        cmdList->CommitShaderResources();

                        cmdList->Dispatch(Math::Max((u32)1, currentScope->groupCountX),
                            Math::Max((u32)1, currentScope->groupCountY),
                            Math::Max((u32)1, currentScope->groupCountZ));
                    }
                }
                else if (currentScope->queueClass == RHI::HardwareQueueClass::Transfer)
                {
                    // TODO: Add transfer pass
                }
            }
        }
        cmdList->End();
        
        RHI::CommandQueueSubmission submission{};
        submission.numCommandLists = 1;
        submission.commandLists = (RHI::CommandList**)&cmdList;
        submission.numPresentSwapChains = scopeChain.Top()->presentSwapChains.GetSize();
        submission.presentSwapChains = scopeChain.Top()->presentSwapChains.GetData();
        
        if (frameGraph->GetEndScopes().Exists(scope)) // A terminal scope
        {
            submission.signalFence = renderingFinishedFence;
            submission.signalFenceValue = renderingFinishedFence->NextSignalValue();
            
            renderingFinishedValues[currentSubmissionIndex] = submission.signalFenceValue;
        }
        
        scope->queue->Submit(submission);
        
        return true;
    }
    
    void FrameGraphExecuter::ResetFramesInFlight()
    {
        ZoneScoped;
        
    }
    
    bool FrameGraphExecuter::ExecuteInternal(const FrameGraphExecuteRequest& executeRequest)
    {
        ZoneScoped;
        
    }
    
}
