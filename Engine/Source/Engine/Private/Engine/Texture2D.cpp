#include "Engine.h"

namespace CE
{
	Texture2D::Texture2D()
	{
		dimension = TextureDimension::Tex2D;
	}

	RPI::Texture* Texture2D::GetRpiTexture()
    {
		ZoneScoped;

		if (!rpiTexture)
		{
			RPI::TextureDescriptor desc{};
			desc.samplerDesc.addressModeU = TextureAddressModeToSamplerMode(addressModeU);
			desc.samplerDesc.addressModeV = TextureAddressModeToSamplerMode(addressModeV);
			desc.samplerDesc.addressModeW = desc.samplerDesc.addressModeU;
			
			desc.texture.width = width;
			desc.texture.height = height;
			desc.texture.depth = 1;
			desc.texture.arrayLayers = arrayCount;
			desc.texture.name = GetName();
			desc.texture.bindFlags = RHI::TextureBindFlags::ShaderRead;
			desc.texture.mipLevels = mipLevels;
			desc.texture.sampleCount = 1;
			desc.texture.dimension = Dimension::Dim2D;
			
			if (TextureSourceCompressionFormatIsBCn(sourceCompressionFormat))
			{
				switch (sourceCompressionFormat)
				{
				case TextureSourceCompressionFormat::BC1:
					desc.texture.format = RHI::Format::BC1_RGB_UNORM;
					if (colorSpace == TextureColorSpace::SRGB)
						desc.texture.format = RHI::Format::BC1_RGB_SRGB;
					break;
				case TextureSourceCompressionFormat::BC3:
					desc.texture.format = RHI::Format::BC3_UNORM;
					if (colorSpace == TextureColorSpace::SRGB)
						desc.texture.format = RHI::Format::BC3_SRGB;
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
					if (colorSpace == TextureColorSpace::SRGB)
						desc.texture.format = RHI::Format::BC7_SRGB;
					break;
				}
			}
			else
			{
				desc.texture.format = ToRHIFormat(pixelFormat, colorSpace == TextureColorSpace::SRGB);
			}
			
			desc.source = &source;

			rpiTexture = new RPI::Texture(desc);
		}

		return rpiTexture;
    }

	RPI::Texture* Texture2D::CloneRpiTexture()
	{
		ZoneScoped;

		RPI::TextureDescriptor desc{};
		desc.samplerDesc.addressModeU = TextureAddressModeToSamplerMode(addressModeU);
		desc.samplerDesc.addressModeV = TextureAddressModeToSamplerMode(addressModeV);
		desc.samplerDesc.addressModeW = desc.samplerDesc.addressModeU;

		desc.texture.width = width;
		desc.texture.height = height;
		desc.texture.depth = 1;
		desc.texture.arrayLayers = arrayCount;
		desc.texture.name = GetName();
		desc.texture.bindFlags = RHI::TextureBindFlags::ShaderRead;
		desc.texture.mipLevels = mipLevels;
		desc.texture.sampleCount = 1;
		desc.texture.dimension = Dimension::Dim2D;

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

	CMImage Texture2D::GetCMImage()
	{
		RHI::Format rhiFormat = ToRHIFormat(GetPixelFormat());
		
		CMImage result = CMImage::LoadRawImageFromMemory(source.GetDataPtr(), width, height, GetCMPixelFormat(), 
			CMImageSourceFormat::None, GetBitDepthForFormat(rhiFormat), GetBitsPerPixelForFormat(rhiFormat));

		return result;
	}

	Ref<CE::Texture2D> Texture2D::Create(Ref<Object> outer, const String& name, const CMImage& sourceImage,
		TextureAddressMode addressModeU, TextureAddressMode addressModeV)
	{
		Ref<CE::Texture2D> texture = CreateObject<CE::Texture2D>(outer.Get(), FixObjectName(name));
		texture->source.LoadData(sourceImage.GetDataPtr(), sourceImage.GetDataSize());

		texture->width = sourceImage.GetWidth();
		texture->height = sourceImage.GetHeight();
		texture->dimension = TextureDimension::Tex2D;
		texture->arrayCount = 1;
		texture->mipLevels = 1;
		texture->addressModeU = addressModeU;
		texture->addressModeV = addressModeV;
		texture->filter = RHI::FilterMode::Linear;
		texture->pixelFormat = ToTextureFormat(sourceImage.GetFormat());

		texture->compressionQuality = TextureCompressionQuality::Default;

		switch (sourceImage.GetFormat())
		{
		case CMImageFormat::BC1:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::BC1;
			break;
		case CMImageFormat::BC3:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::BC3;
			break;
		case CMImageFormat::BC4:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::BC4;
			break;
		case CMImageFormat::BC5:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::BC5;
			break;
		case CMImageFormat::BC6H:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::BC6H;
			break;
		case CMImageFormat::BC7:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::BC7;
			break;
		default:
			texture->sourceCompressionFormat = TextureSourceCompressionFormat::None;
			break;
		}

		return texture;
	}
} // namespace CE

