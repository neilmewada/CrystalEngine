#include "Engine.h"

namespace CE
{
    CE::TextureCube::TextureCube()
    {
        dimension = TextureDimension::TexCube;
    }

    CE::TextureCube::~TextureCube()
    {

    }

    RPI::Texture* CE::TextureCube::GetRpiTexture()
    {
        if (!rpiTexture)
        {
            RPI::TextureDescriptor desc{};
            desc.samplerDesc.addressModeU = TextureAddressModeToSamplerMode(addressModeU);
            desc.samplerDesc.addressModeV = TextureAddressModeToSamplerMode(addressModeV);
            desc.samplerDesc.addressModeW = desc.samplerDesc.addressModeU;
            desc.samplerDesc.enableAnisotropy = anisoLevel > 0;
            desc.samplerDesc.maxAnisotropy = anisoLevel;
            desc.samplerDesc.samplerFilterMode = filter;
            desc.samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;

            desc.texture.width = width;
            desc.texture.height = height;
            desc.texture.depth = 1;
            desc.texture.arrayLayers = 6;
            desc.texture.name = GetName();
            desc.texture.bindFlags = RHI::TextureBindFlags::ShaderRead;
            desc.texture.mipLevels = mipLevels;
            desc.texture.sampleCount = 1;
            desc.texture.dimension = Dimension::DimCUBE;
            desc.texture.defaultHeapType = MemoryHeapType::Default;
            
            if (TextureSourceCompressionFormatIsBCn(sourceCompressionFormat))
            {
                switch (sourceCompressionFormat)
                {
                case TextureSourceCompressionFormat::BC1:
                    desc.texture.format = RHI::Format::BC1_RGB_UNORM;
                    break;
                case TextureSourceCompressionFormat::BC3:
                    desc.texture.format = RHI::Format::BC3_UNORM;
                    break;
                case TextureSourceCompressionFormat::BC4:
                    desc.texture.format = RHI::Format::BC4_UNORM;
                    break;
                case TextureSourceCompressionFormat::BC5:
                    desc.texture.format = RHI::Format::BC5_UNORM;
                    break;
                case TextureSourceCompressionFormat::BC6H:
                    desc.texture.format = RHI::Format::BC6H_UFLOAT;
                    break;
                case TextureSourceCompressionFormat::BC7:
                    desc.texture.format = RHI::Format::BC7_UNORM;
                    break;
                }
            }
            else
            {
                desc.texture.format = ToRHIFormat(pixelFormat);
            }

            desc.source = &source;

            rpiTexture = new RPI::Texture(desc);
        }
        return rpiTexture;
    }

    RPI::Texture* TextureCube::CloneRpiTexture()
    {
        RPI::TextureDescriptor desc{};
        desc.samplerDesc.addressModeU = TextureAddressModeToSamplerMode(addressModeU);
        desc.samplerDesc.addressModeV = TextureAddressModeToSamplerMode(addressModeV);
        desc.samplerDesc.addressModeW = desc.samplerDesc.addressModeU;
        desc.samplerDesc.enableAnisotropy = anisoLevel > 0;
        desc.samplerDesc.maxAnisotropy = anisoLevel;
        desc.samplerDesc.samplerFilterMode = filter;
        desc.samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;

        desc.texture.width = width;
        desc.texture.height = height;
        desc.texture.depth = 1;
        desc.texture.arrayLayers = 6;
        desc.texture.name = GetName();
        desc.texture.bindFlags = RHI::TextureBindFlags::ShaderRead;
        desc.texture.mipLevels = mipLevels;
        desc.texture.sampleCount = 1;
        desc.texture.dimension = Dimension::DimCUBE;
        desc.texture.defaultHeapType = MemoryHeapType::Default;

        if (TextureSourceCompressionFormatIsBCn(sourceCompressionFormat))
        {
            switch (sourceCompressionFormat)
            {
            case TextureSourceCompressionFormat::BC1:
                desc.texture.format = RHI::Format::BC1_RGB_UNORM;
                break;
            case TextureSourceCompressionFormat::BC3:
                desc.texture.format = RHI::Format::BC3_UNORM;
                break;
            case TextureSourceCompressionFormat::BC4:
                desc.texture.format = RHI::Format::BC4_UNORM;
                break;
            case TextureSourceCompressionFormat::BC5:
                desc.texture.format = RHI::Format::BC5_UNORM;
                break;
            case TextureSourceCompressionFormat::BC6H:
                desc.texture.format = RHI::Format::BC6H_UFLOAT;
                break;
            case TextureSourceCompressionFormat::BC7:
                desc.texture.format = RHI::Format::BC7_UNORM;
                break;
            }
        }
        else
        {
            desc.texture.format = ToRHIFormat(pixelFormat);
        }

        desc.source = &source;

        return new RPI::Texture(desc);
    }

} // namespace CE
