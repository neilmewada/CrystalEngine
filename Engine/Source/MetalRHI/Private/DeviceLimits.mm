#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{

    DeviceLimits::DeviceLimits(Device* device) : device(device)
    {
        for (const TextureFormatEntry& entry : formatEntries)
        {
            RHI::TextureBindFlags bindFlags = RHI::TextureBindFlags::None;
            
            if (IsDepthMtlFormat(entry.mtlFormat))
                bindFlags |= RHI::TextureBindFlags::Depth;
            if (IsStencilMtlFormat(entry.mtlFormat))
                bindFlags |= RHI::TextureBindFlags::DepthStencil;
            
            if (bindFlags == RHI::TextureBindFlags::None)
                bindFlags = RHI::TextureBindFlags::Color;
            
            bindFlags |= RHI::TextureBindFlags::ShaderReadWrite | RHI::TextureBindFlags::SubpassInput;
            
            imageFormatSupport[entry.rhiFormat] = FormatEntry{
                .bindFlags = bindFlags,
                .filterMask = RHI::FilterModeMask::Nearest | RHI::FilterModeMask::Linear
            };
        }
        
        maxStructuredBufferRange = NumericLimits<u32>::Max();
        maxConstantBufferRange = 16 * 1024 * 1024; // 16 MB max
    }

    DeviceLimits::~DeviceLimits()
    {
        
    }
    
} // namespace CE::Metal
