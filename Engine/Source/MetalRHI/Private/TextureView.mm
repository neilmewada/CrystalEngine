#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{

    TextureView::TextureView(Device* device, const RHI::TextureViewDescriptor& desc)
        : RHI::TextureView(desc), device(device)
    {
        texture = (Metal::Texture*)desc.texture;
        
        mtlFormat = ToMtlFormat(desc.format);
        
        switch (desc.dimension)
        {
            case RHI::Dimension::Dim2D:
                mtlTextureType = MTLTextureType2D;
                break;
            case RHI::Dimension::DimCUBE:
                mtlTextureType = MTLTextureTypeCube;
                break;
            case RHI::Dimension::Dim3D:
                mtlTextureType = MTLTextureType3D;
                break;
            case RHI::Dimension::Dim1D:
                mtlTextureType = MTLTextureType1D;
                break;
            case RHI::Dimension::Dim2DArray:
                mtlTextureType = MTLTextureType2DArray;
                break;
        }
        
        mtlTextureView = [texture->GetMtlTexture() newTextureViewWithPixelFormat:mtlFormat
                                                                     textureType:mtlTextureType
                                                                          levels:NSMakeRange(desc.baseArrayLayer, desc.arrayLayerCount)
                                                                          slices:NSMakeRange(desc.baseMipLevel, desc.mipLevelCount)];
    }

    TextureView::~TextureView()
    {
        mtlTextureView = nil;
    }
    
} // namespace CE::Metal
    
