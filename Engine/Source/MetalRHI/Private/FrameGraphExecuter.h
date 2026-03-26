#pragma once

namespace CE::Metal
{
    class Scope;
    
    class FrameGraphExecuter : public RHI::FrameGraphExecuter
    {
    public:
        
        FrameGraphExecuter(Device* device);
        virtual ~FrameGraphExecuter();
        
        void WaitUntilIdle() override;
        
        u32 BeginExecution(const FrameGraphExecuteRequest &executeRequest) override;
        
        void EndExecution(const FrameGraphExecuteRequest &executeRequest) override;
        
        void ResetFramesInFlight() override;

        bool ExecuteScope(const RHI::FrameGraphExecuteRequest& executeRequest, Metal::Scope* scope,
                          HashSet<RHI::ScopeId>& executedScopes,
                          HashSet<Metal::SwapChain*>& usedSwapChains);
        
    private:
        
        Device* device = nullptr;
        
        Metal::FrameGraphCompiler* compiler = nullptr;
        
        uint64_t renderingFinishedValues[RHI::Limits::MaxSwapChainImageCount] = {};
        RHI::Fence* renderingFinishedFence = nullptr;
        
        u32 currentSubmissionIndex = 0;
    };
    
} // namespace CE::Metal
