#pragma once

namespace CE::Metal
{
    
    class FrameGraphExecuter : public RHI::FrameGraphExecuter
    {
    public:
        
        FrameGraphExecuter(Device* device);
        virtual ~FrameGraphExecuter();
        
        void WaitUntilIdle() override;
        
        u32 BeginExecution(const FrameGraphExecuteRequest &executeRequest) override;
        
        void EndExecution(const FrameGraphExecuteRequest &executeRequest) override;
        
        void ResetFramesInFlight() override;
        
        bool ExecuteInternal(const FrameGraphExecuteRequest &executeRequest) override;
        
        
    private:
        
        Device* device = nullptr;
    };
    
} // namespace CE::Metal
