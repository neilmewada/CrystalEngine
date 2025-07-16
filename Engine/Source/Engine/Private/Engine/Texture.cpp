#include "Engine.h"

namespace CE
{
	ENGINE_API RHI::Format CE::ToRHIFormat(CE::TextureFormat format, bool isSrgb)
	{
		switch (format)
		{
		case TextureFormat::RGBA8:
			return isSrgb ? RHI::Format::R8G8B8A8_SRGB : RHI::Format::R8G8B8A8_UNORM;
		case TextureFormat::RGB8:
			return isSrgb ? RHI::Format::R8G8B8_SRGB : RHI::Format::R8G8B8_UNORM;
		case TextureFormat::BGRA8:
			return isSrgb ? RHI::Format::B8G8R8A8_SRGB : RHI::Format::B8G8R8A8_UNORM;
		case TextureFormat::BGR8:
			return isSrgb ? RHI::Format::B8G8R8_SRGB : RHI::Format::B8G8R8_UNORM;
		case TextureFormat::R8:
			return isSrgb ? RHI::Format::R8_SRGB : RHI::Format::R8_UNORM;
		case TextureFormat::R16:
			return RHI::Format::R16_UNORM;
		case TextureFormat::RG8:
			return isSrgb ? RHI::Format::R8G8_SRGB : RHI::Format::R8G8_UNORM;
		case TextureFormat::RG16:
			return RHI::Format::R16G16_UNORM;
		case TextureFormat::RHalf:
			return RHI::Format::R16_SFLOAT;
		case TextureFormat::RGHalf:
			return RHI::Format::R16G16_SFLOAT;
		case TextureFormat::RGBAHalf:
			return RHI::Format::R16G16B16A16_SFLOAT;
		case TextureFormat::RFloat:
			return RHI::Format::R32_SFLOAT;
		case TextureFormat::RGFloat:
			return RHI::Format::R32G32_SFLOAT;
		case TextureFormat::RGBAFloat:
			return RHI::Format::R32G32B32A32_SFLOAT;
		case TextureFormat::RGB565:
			return RHI::Format::R5G6B5_UNORM;
		case TextureFormat::BC1:
			return isSrgb ? RHI::Format::BC1_RGB_SRGB : Format::BC1_RGB_UNORM;
		case TextureFormat::BC3:
			return isSrgb ? RHI::Format::BC3_SRGB : RHI::Format::BC3_UNORM;
		case TextureFormat::BC4:
			return RHI::Format::BC4_UNORM;
		case TextureFormat::BC5:
			return RHI::Format::BC5_UNORM;
		case TextureFormat::BC6H:
			return RHI::Format::BC6H_UFLOAT;
		case TextureFormat::BC7:
			return isSrgb ? RHI::Format::BC7_SRGB : Format::BC7_UNORM;
		}
		return RHI::Format::Undefined;
	}

	CE::TextureFormat ToTextureFormat(CMImageFormat format)
	{
		switch (format)
		{

		case CMImageFormat::R8:
			return TextureFormat::R8;
		case CMImageFormat::RG8:
			return TextureFormat::RG8;
		case CMImageFormat::RGB8:
			return TextureFormat::RGB8;
		case CMImageFormat::RGBA8:
			return TextureFormat::RGBA8;
		case CMImageFormat::R32:
			return TextureFormat::RFloat;
		case CMImageFormat::RG32:
			return TextureFormat::RGFloat;
		case CMImageFormat::RGBA32:
			return TextureFormat::RGBAFloat;
		case CMImageFormat::R16:
			return TextureFormat::R16;
		case CMImageFormat::RG16:
			return TextureFormat::RG16;

		case CMImageFormat::RGB565:
			return TextureFormat::RGB565;
		case CMImageFormat::BC1:
			return TextureFormat::BC1;
		case CMImageFormat::BC3:
			return TextureFormat::BC3;
		case CMImageFormat::BC4:
			return TextureFormat::BC4;
		case CMImageFormat::BC5:
			return TextureFormat::BC5;
		case CMImageFormat::BC6H:
			return TextureFormat::BC6H;
		case CMImageFormat::BC7:
			return TextureFormat::BC7;

		case CMImageFormat::Undefined:
		case CMImageFormat::RGB32:
		case CMImageFormat::RGB16:
		case CMImageFormat::RGBA16:
			return TextureFormat::None;
		}

		return TextureFormat::None;
	}

	ENGINE_API bool TextureSourceCompressionFormatIsBCn(TextureSourceCompressionFormat sourceCompressionFormat)
	{
		switch (sourceCompressionFormat)
		{
		case TextureSourceCompressionFormat::BC1:
		case TextureSourceCompressionFormat::BC3:
		case TextureSourceCompressionFormat::BC4:
		case TextureSourceCompressionFormat::BC5:
		case TextureSourceCompressionFormat::BC6H:
		case TextureSourceCompressionFormat::BC7:
			return true;
		}

		return false;
	}

	CE::Texture::Texture()
	{
		
	}

	CE::Texture::~Texture()
	{
		delete rpiTexture; rpiTexture = nullptr;
	}

	CMImageFormat CE::Texture::GetCMPixelFormat()
	{
		switch (pixelFormat)
		{
		case TextureFormat::R8:
			return CMImageFormat::R8;
		case TextureFormat::R16:
		case TextureFormat::RHalf:
			return CMImageFormat::R16;
		case TextureFormat::RG8:
			return CMImageFormat::RG8;
		case TextureFormat::RG16:
		case TextureFormat::RGHalf:
			return CMImageFormat::RG16;
		case TextureFormat::RGB8:
			return CMImageFormat::RGB8;
		case TextureFormat::BC7:
			return CMImageFormat::BC7;
		case TextureFormat::BC1:
			return CMImageFormat::BC1;
		case TextureFormat::BC3:
			return CMImageFormat::BC3;
		case TextureFormat::BC4:
			return CMImageFormat::BC4;
		case TextureFormat::BC5:
			return CMImageFormat::BC5;
		case TextureFormat::BC6H:
			return CMImageFormat::BC6H;
		case TextureFormat::None:
			break;
		case TextureFormat::RGBA8:
			return CMImageFormat::RGBA8;
		case TextureFormat::BGRA8:
			return CMImageFormat::RGBA8;
		case TextureFormat::BGR8:
			return CMImageFormat::RGB8;
		case TextureFormat::RGB565:
			return CMImageFormat::RGB565;
		case TextureFormat::RGBAHalf:
			return CMImageFormat::RGBA16;
		case TextureFormat::RFloat:
			return CMImageFormat::R32;
		case TextureFormat::RGFloat:
			return CMImageFormat::RG32;
		case TextureFormat::RGBAFloat:
			return CMImageFormat::RGBA32;
		}
		return CMImageFormat::Undefined;
	}

	ENGINE_API RHI::SamplerAddressMode TextureAddressModeToSamplerMode(TextureAddressMode mode)
	{
		switch (mode)
		{
		case TextureAddressMode::Repeat:
			return RHI::SamplerAddressMode::Repeat;
		case TextureAddressMode::Clamp:
			return RHI::SamplerAddressMode::ClampToEdge;
		case TextureAddressMode::ClampToBorder:
			return RHI::SamplerAddressMode::ClampToBorder;
		}

		return RHI::SamplerAddressMode::Repeat;
	}

} // namespace CE


