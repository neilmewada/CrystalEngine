#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    FrameGraphExecuter::FrameGraphExecuter(Device* device) : device(device)
    {
        
    }
    
    FrameGraphCompiler::~FrameGraphCompiler()
    {
        
    }
    
    void FrameGraphExecuter::WaitUntilIdle()
    {
        ZoneScoped;
        
    }
    
    u32 FrameGraphExecuter::BeginExecution(const FrameGraphExecuteRequest& executeRequest)
    {
        ZoneScoped;
        
        return 0;
    }
    
    void FrameGraphExecuter::EndExecution(const FrameGraphExecuteRequest& executeRequest)
    {
        ZoneScoped;
        
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
