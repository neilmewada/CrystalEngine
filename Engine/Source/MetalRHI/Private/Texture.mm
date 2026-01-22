#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{
    MTLTextureDescriptor* ToMtlTextureDescriptor(const RHI::TextureDescriptor& desc)
    {
        MTLTextureDescriptor* textureDesc = [[MTLTextureDescriptor alloc] init];
        
        textureDesc.width = desc.width;
        textureDesc.height = desc.height;
        textureDesc.depth = desc.depth;
        
        switch (desc.dimension)
        {
            case RHI::Dimension::Dim2D:
                textureDesc.textureType = MTLTextureType2D;
                break;
            case RHI::Dimension::DimCUBE:
                textureDesc.textureType = MTLTextureTypeCube;
                break;
            case RHI::Dimension::Dim3D:
                textureDesc.textureType = MTLTextureType3D;
                break;
            case RHI::Dimension::Dim1D:
                textureDesc.textureType = MTLTextureType1D;
                break;
            case RHI::Dimension::Dim2DArray:
                textureDesc.textureType = MTLTextureType2DArray;
                break;
        }
        
        textureDesc.mipmapLevelCount = desc.mipLevels;
        textureDesc.arrayLength = desc.arrayLayers;
        
        textureDesc.sampleCount = desc.sampleCount;
        
        switch (desc.defaultHeapType)
        {
            case RHI::MemoryHeapType::Default:
                textureDesc.storageMode = MTLStorageModePrivate;
                break;
            case RHI::MemoryHeapType::Upload:
                textureDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
            case RHI::MemoryHeapType::ReadBack:
                textureDesc.allowGPUOptimizedContents = NO;
                textureDesc.storageMode = MTLStorageModeShared;
                break;
        }
        
#if CE_BUILD_DEBUG
        textureDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
#endif
        
        textureDesc.usage = MTLTextureUsageUnknown;
        
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderRead))
        {
            textureDesc.usage |= MTLTextureUsageShaderRead;
        }
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderWrite))
        {
            textureDesc.usage |= MTLTextureUsageShaderWrite;
        }
        if (EnumHasAnyFlags(desc.bindFlags, RHI::TextureBindFlags::Depth | RHI::TextureBindFlags::Color | RHI::TextureBindFlags::DepthStencil))
        {
            textureDesc.usage |= MTLTextureUsageRenderTarget;
        }
        
        return textureDesc;
    }
    
    void MetalRHI::GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements)
    {
        MTLTextureDescriptor* desc = ToMtlTextureDescriptor(textureDesc);
        
        MTLSizeAndAlign req = [device->GetHandle() heapTextureSizeAndAlignWithDescriptor:desc];
        
        outRequirements.size = req.size;
        outRequirements.offsetAlignment = req.align;
    }

    Texture::Texture(Device* device, const RHI::TextureDescriptor& desc)
        : device(device)
    {
        Init(desc);
        
        mtlTexture = [device->GetHandle() newTextureWithDescriptor:textureDesc];
    }

    Texture::Texture(Device* device, const RHI::TextureDescriptor& desc, const RHI::ResourceMemoryDescriptor& memoryDesc)
        : device(device)
    {
        Init(desc);
        
        memoryHeap = (Metal::MemoryHeap*)memoryDesc.memoryHeap;
        memoryOffset = memoryDesc.memoryOffset;
        
        mtlTexture = [memoryHeap->GetMtlHeap() newTextureWithDescriptor:textureDesc offset:memoryOffset];
    }

    u32 Texture::GetNumberOfChannels()
    {
        return RHI::GetNumChannelsForFormat(format);
    }

    u32 Texture::GetBitsPerPixel()
    {
        return RHI::GetBitsPerPixelForFormat(format);
    }

    void Texture::Init(const RHI::TextureDescriptor& desc)
    {
        LoadMappings();
        
        this->name = desc.name;
        this->width = desc.width;
        this->height = desc.height;
        this->depth = desc.depth;
        this->arrayLayers = desc.arrayLayers;

        this->dimension = desc.dimension;
        this->format = desc.format;
        this->mipLevels = desc.mipLevels;
        this->sampleCount = desc.sampleCount;
        
        textureDesc = [[MTLTextureDescriptor alloc] init];
        
        textureDesc.width = width;
        textureDesc.height = height;
        textureDesc.depth = depth;
        
        switch (dimension)
        {
            case RHI::Dimension::Dim2D:
                textureDesc.textureType = MTLTextureType2D;
                break;
            case RHI::Dimension::DimCUBE:
                textureDesc.textureType = MTLTextureTypeCube;
                break;
            case RHI::Dimension::Dim3D:
                textureDesc.textureType = MTLTextureType3D;
                break;
            case RHI::Dimension::Dim1D:
                textureDesc.textureType = MTLTextureType1D;
                break;
            case RHI::Dimension::Dim2DArray:
                textureDesc.textureType = MTLTextureType2DArray;
                break;
        }
        
        textureDesc.mipmapLevelCount = desc.mipLevels;
        textureDesc.arrayLength = desc.arrayLayers;
        
        textureDesc.sampleCount = desc.sampleCount;
        
        switch (desc.defaultHeapType)
        {
            case RHI::MemoryHeapType::Default:
                textureDesc.storageMode = MTLStorageModePrivate;
                break;
            case RHI::MemoryHeapType::Upload:
                textureDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
            case RHI::MemoryHeapType::ReadBack:
                textureDesc.allowGPUOptimizedContents = NO;
                textureDesc.storageMode = MTLStorageModeShared;
                break;
        }
        
#if CE_BUILD_DEBUG
        textureDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
#endif
        
        textureDesc.usage = MTLTextureUsageUnknown;
        
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderRead))
        {
            textureDesc.usage |= MTLTextureUsageShaderRead;
        }
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderWrite))
        {
            textureDesc.usage |= MTLTextureUsageShaderWrite;
        }
        if (EnumHasAnyFlags(desc.bindFlags, RHI::TextureBindFlags::Depth | RHI::TextureBindFlags::Color | RHI::TextureBindFlags::DepthStencil))
        {
            textureDesc.usage |= MTLTextureUsageRenderTarget;
        }
        
        
    }

}
