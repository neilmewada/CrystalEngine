#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    FrameGraphCompiler::FrameGraphCompiler(Device* device) : device(device)
    {
        
    }
    
    FrameGraphCompiler::~FrameGraphCompiler()
    {
        
    }
    
    void FrameGraphCompiler::CompileScopesInternal(const FrameGraphCompileRequest& compileRequest)
    {
        ZoneScoped;
        
        numFramesInFlight = compileRequest.numFramesInFlight;
        
        RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
        
        // TODO: Allocate different queues for parallel command execution
        
        std::function<void(RHI::Scope*, int)> visitor = [&](RHI::Scope* rhiScope, int i)
        {
            auto scope = (Metal::Scope*)rhiScope;
            
            bool swapChainFound = false;
            
            for (auto attachment : scope->attachments)
            {
                RHI::FrameAttachment* frameAttachment = attachment->GetFrameAttachment();
                if (frameAttachment->IsSwapChainAttachment())
                {
                    swapChainFound = true;
                    RHI::SwapChain* swapChain = ((RHI::SwapChainFrameAttachment*)frameAttachment)->GetSwapChain();
                    if (!scope->swapChainsUsedByAttachments.Exists(swapChain))
                        scope->swapChainsUsedByAttachments.Add(swapChain);
                }
            }
            
            if (!swapChainFound)
            {
                scope->swapChainsUsedByAttachments.Clear();
            }
            
            for (auto consumer : frameGraph->nodes[scope->GetId()].consumers)
            {
                visitor(consumer, i);
                i++;
            }
        };
        
        int trackNumber = 0;
        for (RHI::Scope* scope : frameGraph->producers)
        {
            visitor(scope, trackNumber);
            trackNumber++;
        }
    }
    
    void FrameGraphCompiler::CompileInternal(const FrameGraphCompileRequest &compileRequest)
    {
        ZoneScoped;
        
        RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
        
        // If frame-graph presents at least 1 swap-chain
        bool presentSwapChains = false;
        
        numFramesInFlight = compileRequest.numFramesInFlight;
        u32 imageCount = numFramesInFlight;

        if (frameGraph->presentSwapChains.NotEmpty())
        {
            presentSwapChains = true;
        }
        
        // Compile individual scopes
        for (auto scope : frameGraph->scopes)
        {
            scope->Compile(compileRequest);
        }
    }
    
}
