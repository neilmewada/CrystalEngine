#pragma once

namespace CE::Metal
{
    
    class METALRHI_API Sampler : public RHI::Sampler
    {
    public:
        Sampler(Device* device, const RHI::SamplerDescriptor& samplerDesc);
        virtual ~Sampler();
        
        void *GetHandle() override
        {
            return (void*)sampler;
        }
        
        id<MTLSamplerState> GetMtlSamplerState() const { return sampler; }
        
    private:
        
        Device* device = nullptr;
        SamplerBorderColor borderColor{};
        
        id<MTLSamplerState> sampler = nil;
    };

} // namespace CE
