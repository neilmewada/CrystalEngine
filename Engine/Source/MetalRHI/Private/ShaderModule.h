#pragma once

namespace CE::Metal
{

    class ShaderModule : public RHI::ShaderModule
    {
    public:

        ShaderModule(Device* device, const RHI::ShaderModuleDescriptor& desc);
        virtual ~ShaderModule();
        
        id<MTLLibrary> GetMtlLibrary() const { return mtlLibrary; }

    private:

        Device* device = nullptr;
        
        id<MTLLibrary> mtlLibrary = nil;
    };
    
} // namespace CE::Metal
