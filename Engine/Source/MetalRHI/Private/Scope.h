#pragma once

namespace CE::Metal
{
    
    class Scope : public RHI::Scope
    {
    public:
        using Super = RHI::Scope;
        using Self = Scope;
        
        Scope(Device* device, const RHI::ScopeDescriptor& desc);
        virtual ~Scope();
        
        bool CompileInternal(const FrameGraphCompileRequest& compileRequest) override;
        
    private:
        
        Device* device = nullptr;
        Metal::CommandQueue* queue = nullptr;
        
        StaticArray<Metal::CommandList*, RHI::Limits::MaxSwapChainImageCount> cmdLists;
        
        Metal::RenderPassFrameBuffer* frameBuffer = nullptr;
        
        Metal::RenderPass* renderPass = nullptr;
        
        RHI::Fence* renderFinishedFence = nullptr;
        uint64_t renderFinishedValue[RHI::Limits::MaxSwapChainImageCount] = {};
        
        friend class FrameGraphCompiler;
        friend class FrameGraphExecuter;
    };
    
} // namespace CE::Metal
