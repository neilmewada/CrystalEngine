#pragma once

namespace CE::Metal
{

    struct TextureFormatEntry
    {
        RHI::Format rhiFormat = RHI::Format::Undefined;
        MTLPixelFormat mtlFormat = MTLPixelFormatInvalid;
        u32 numChannels = 0;
        u32 totalBits = 0;
    };

    thread_local static const Array<TextureFormatEntry> formatEntries{
        // RHI::Format, VkFormat, numChannels, totalBytes
        { RHI::Format::R8_UNORM, MTLPixelFormatR8Unorm, 1, 8 },
        { RHI::Format::R8_SNORM, MTLPixelFormatR8Snorm, 1, 8 },
        { RHI::Format::R8_SRGB, MTLPixelFormatR8Unorm_sRGB, 1, 8 },

        { RHI::Format::R8G8_SRGB, MTLPixelFormatRG8Unorm_sRGB, 2, 16 },
        { RHI::Format::R8G8_UNORM, MTLPixelFormatRG8Unorm, 2, 16 },

        { RHI::Format::R16G16_UNORM, MTLPixelFormatRG16Unorm, 2, 32 },
        { RHI::Format::R16G16_SNORM, MTLPixelFormatRG16Snorm, 2, 32 },
        { RHI::Format::R16G16_SINT, MTLPixelFormatRG16Sint, 2, 32 },
        { RHI::Format::R16G16_SFLOAT, MTLPixelFormatRG16Float, 2, 32 },

        { RHI::Format::R16G16B16A16_SFLOAT, MTLPixelFormatRGBA16Float, 4, 8*8 },
        { RHI::Format::R32G32B32A32_SFLOAT, MTLPixelFormatRGBA32Float, 4, 16*8 },

        { RHI::Format::R32G32_UINT, MTLPixelFormatRG32Uint, 2, 8*8 },
        { RHI::Format::R32G32_SINT, MTLPixelFormatRG32Sint, 2, 8*8 },
        { RHI::Format::R32G32_SFLOAT, MTLPixelFormatRG32Float, 2, 8 * 8 },

        { RHI::Format::R8G8B8A8_SRGB, MTLPixelFormatRGBA8Unorm_sRGB, 4, 4 * 8 },
        { RHI::Format::R8G8B8A8_UNORM, MTLPixelFormatRGBA8Unorm, 4, 4 * 8 },
        { RHI::Format::R8G8B8A8_SNORM, MTLPixelFormatRGBA8Snorm, 4, 4 * 8 },

        //{ RHI::Format::R5G6B5_UNORM, MTLPixelFormatB5G6R5Unorm, 3, 2 * 8 },
        { RHI::Format::B5G6R5_UNORM, MTLPixelFormatB5G6R5Unorm, 3, 2 * 8 },

        { RHI::Format::B8G8R8A8_SRGB, MTLPixelFormatBGRA8Unorm_sRGB, 4, 4 * 8 },
        { RHI::Format::B8G8R8A8_UNORM, MTLPixelFormatBGRA8Unorm, 4, 4 * 8 },
        { RHI::Format::B8G8R8A8_SNORM, MTLPixelFormatBGRA8Unorm, 4, 4 * 8 },

        //{ RHI::Format::R8G8B8_UNORM, MTLPixelFormatRGB8Unorm, 3, 3 * 8 },
        //{ RHI::Format::R8G8B8_SNORM, VK_FORMAT_R8G8B8_SNORM, 3, 3 * 8 },
        //{ RHI::Format::R8G8B8_SRGB, VK_FORMAT_R8G8B8_SRGB, 3, 3 * 8 },

        { RHI::Format::R16_UNORM, MTLPixelFormatR16Unorm, 1, 2 * 8 },
        { RHI::Format::R16_SNORM, MTLPixelFormatR16Snorm, 1, 2 * 8 },
        { RHI::Format::R16_SFLOAT, MTLPixelFormatR16Float, 1, 2 * 8 },
        { RHI::Format::R32_UINT, MTLPixelFormatR32Uint, 1, 4 * 8 },
        { RHI::Format::R32_SINT, MTLPixelFormatR32Sint, 1, 4 * 8 },
        { RHI::Format::R32_SFLOAT, MTLPixelFormatR32Float, 1, 4 * 8 },
        { RHI::Format::D32_SFLOAT, MTLPixelFormatDepth32Float, 1, 4 * 8 },
        { RHI::Format::D32_SFLOAT_S8_UINT, MTLPixelFormatDepth32Float_Stencil8, 1, 32 + 8 },
        { RHI::Format::D24_UNORM_S8_UINT, MTLPixelFormatDepth24Unorm_Stencil8, 1, 24 + 8 },
        //{ RHI::Format::D16_UNORM_S8_UINT, MTLPixelFormatDepth16Unorm, 1, 16 + 8 },
        { RHI::Format::D16_UNORM, MTLPixelFormatDepth16Unorm, 1, 16 },

        //{ RHI::Format::B8G8R8_UNORM, MTLPixelFormatBGRA8Unorm, 3, 3 * 8 },
        //{ RHI::Format::B8G8R8_SNORM, VK_FORMAT_B8G8R8_SNORM, 3, 3 * 8 },
        //{ RHI::Format::B8G8R8_SRGB, VK_FORMAT_B8G8R8_SRGB, 3, 3 * 8 },

        { RHI::Format::BC1_RGB_UNORM, MTLPixelFormatBC1_RGBA, 3, 4 },
        { RHI::Format::BC1_RGBA_UNORM, MTLPixelFormatBC1_RGBA, 4, 4 },
        { RHI::Format::BC1_RGB_SRGB, MTLPixelFormatBC1_RGBA_sRGB, 3, 4 },
        { RHI::Format::BC1_RGBA_SRGB, MTLPixelFormatBC1_RGBA_sRGB, 4, 4 },

        { RHI::Format::BC3_SRGB, MTLPixelFormatBC3_RGBA_sRGB, 4, 8 },
        { RHI::Format::BC3_UNORM, MTLPixelFormatBC3_RGBA, 4, 8 },
        { RHI::Format::BC5_UNORM, MTLPixelFormatBC5_RGUnorm, 2, 8 },

        { RHI::Format::BC7_UNORM, MTLPixelFormatBC7_RGBAUnorm, 4, 8 },
        { RHI::Format::BC7_SRGB, MTLPixelFormatBC7_RGBAUnorm_sRGB, 4, 8 },
        { RHI::Format::BC4_UNORM, MTLPixelFormatBC4_RUnorm, 1, 4 },
        { RHI::Format::BC6H_UFLOAT, MTLPixelFormatBC6H_RGBUfloat, 4, 8 },
        { RHI::Format::BC6H_SFLOAT, MTLPixelFormatBC6H_RGBFloat, 4, 8 },
    };

    thread_local static HashMap<RHI::Format, TextureFormatEntry> textureFormatToMtlFormatMap{};
    thread_local static HashMap<MTLPixelFormat, TextureFormatEntry> mtlFormatToTextureFormatMap{};

    inline void LoadMappings()
    {
        if (mtlFormatToTextureFormatMap.GetSize() == 0)
        {
            for (const auto& entry : formatEntries)
            {
                textureFormatToMtlFormatMap[entry.rhiFormat] = entry;
                mtlFormatToTextureFormatMap[entry.mtlFormat] = entry;
            }
        }
    }

    inline MTLPixelFormat ToMtlFormat(RHI::Format format)
    {
        LoadMappings();
        return textureFormatToMtlFormatMap[format].mtlFormat;
    }

    inline RHI::Format ToRhiFormat(MTLPixelFormat mtlFormat)
    {
        LoadMappings();
        return mtlFormatToTextureFormatMap[mtlFormat].rhiFormat;
    }

    inline bool IsDepthMtlFormat(MTLPixelFormat format)
    {
        switch (format) {
            case MTLPixelFormatDepth16Unorm:
            case MTLPixelFormatDepth32Float:
            case MTLPixelFormatDepth24Unorm_Stencil8:
            case MTLPixelFormatDepth32Float_Stencil8:
                return true;
            default:
                return false;
        }
    }

    inline bool IsStencilMtlFormat(MTLPixelFormat format)
    {
        switch (format) {
            case MTLPixelFormatDepth24Unorm_Stencil8:
            case MTLPixelFormatDepth32Float_Stencil8:
            case MTLPixelFormatStencil8:
            case MTLPixelFormatX24_Stencil8:
            case MTLPixelFormatX32_Stencil8:
                return true;
            default:
                return false;
        }
    }

} // namespace CE::Metal

