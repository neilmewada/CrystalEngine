
#include "MetalRHIPrivate.h"


namespace CE::Metal
{
    
    static MTLSamplerAddressMode ToMTLSamplerAddressMode(RHI::SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
            case RHI::SamplerAddressMode::Repeat:
                return MTLSamplerAddressModeRepeat;
            case RHI::SamplerAddressMode::MirroredRepeat:
                return MTLSamplerAddressModeMirrorRepeat;
            case RHI::SamplerAddressMode::ClampToEdge:
                return MTLSamplerAddressModeClampToEdge;
            case RHI::SamplerAddressMode::ClampToBorder:
                return MTLSamplerAddressModeClampToBorderColor;
        }
        
        return MTLSamplerAddressModeRepeat;
    }

    static MTLSamplerMinMagFilter ToMTLSamplerMinMagFilter(RHI::FilterMode filter)
    {
        switch (filter)
        {
            case RHI::FilterMode::Cubic:
            case RHI::FilterMode::Linear:
                return MTLSamplerMinMagFilterLinear;
            case RHI::FilterMode::Nearest:
                return MTLSamplerMinMagFilterNearest;
        }
    }

    static MTLSamplerMipFilter ToMTLSamplerMipFilter(RHI::FilterMode filter)
    {
        switch (filter)
        {
            case RHI::FilterMode::Linear:
            case RHI::FilterMode::Cubic:
                return MTLSamplerMipFilterLinear;
            case RHI::FilterMode::Nearest:
                return MTLSamplerMipFilterNearest;
        }
    }

    Sampler::Sampler(Device* device, const RHI::SamplerDescriptor& samplerDesc)
        : device(device), borderColor(samplerDesc.borderColor)
    {
        MTLSamplerDescriptor* desc = [[MTLSamplerDescriptor alloc] init];
        
        switch (samplerDesc.borderColor)
        {
            case RHI::SamplerBorderColor::FloatTransparentBlack:
            case RHI::SamplerBorderColor::IntTransparentBlack:
                desc.borderColor = MTLSamplerBorderColorTransparentBlack;
                break;
            case RHI::SamplerBorderColor::FloatOpaqueBlack:
            case RHI::SamplerBorderColor::IntOpaqueBlack:
                desc.borderColor = MTLSamplerBorderColorOpaqueBlack;
                break;
            case RHI::SamplerBorderColor::FloatOpaqueWhite:
            case RHI::SamplerBorderColor::IntOpaqueWhite:
                desc.borderColor = MTLSamplerBorderColorOpaqueWhite;
                break;
        }
        
        desc.sAddressMode = ToMTLSamplerAddressMode(samplerDesc.addressModeU);
        desc.tAddressMode = ToMTLSamplerAddressMode(samplerDesc.addressModeV);
        desc.rAddressMode = ToMTLSamplerAddressMode(samplerDesc.addressModeW);
        
        desc.maxAnisotropy =  samplerDesc.enableAnisotropy ? samplerDesc.maxAnisotropy : 1;
        desc.normalizedCoordinates = YES;
        
        desc.minFilter = desc.magFilter = ToMTLSamplerMinMagFilter(samplerDesc.samplerFilterMode);
        desc.mipFilter = ToMTLSamplerMipFilter(samplerDesc.samplerFilterMode);
        
        desc.lodMinClamp = 0.0f;
        desc.lodMaxClamp = FLT_MAX;
        
        sampler = [device->GetHandle() newSamplerStateWithDescriptor:desc];
		
        if (sampler == nil)
        {
            CE_LOG(Error, All, "Failed to create vulkan sampler");
        }
    }

    Sampler::~Sampler()
    {
        sampler = nil;
    }

} // namespace CE

