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
        
        bool CompileInternal(const FrameGraphCompileRequest &compileRequest) override;
        
    private:
        
        Device* device = nullptr;
    };
    
} // namespace CE::Metal
