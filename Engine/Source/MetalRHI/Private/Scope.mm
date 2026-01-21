#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    Scope::Scope(Device* device, const RHI::ScopeDescriptor& desc) : RHI::Scope(desc), device(device)
    {
        
    }
    
    Scope::~Scope()
    {
        
    }
    
    bool Scope::CompileInternal(const FrameGraphCompileRequest &compileRequest)
    {
        
        
        return true;
    }
    
}
