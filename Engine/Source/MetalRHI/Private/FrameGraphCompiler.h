#pragma once

namespace CE::Metal
{
    class Scope;
    
    class FrameGraphCompiler : public RHI::FrameGraphCompiler
    {
    public:
        FrameGraphCompiler(Device* device);
        virtual ~FrameGraphCompiler();
        
        void CompileScopesInternal(const FrameGraphCompileRequest& compileRequest) override;
        
        void CompileInternal(const FrameGraphCompileRequest& compileRequest) override;
        
    private:
        
        Device* device = nullptr;
        
        u32 numFramesInFlight = 2;
        
        friend class FrameGraphExecuter;
    };
    
} // namespace CE::Metal
