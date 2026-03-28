#include "CoreAssets.h"

#if PLATFORM_DESKTOP
#include "ispc_texcomp.h"
#endif

namespace CE
{
	// Class used to store functions to specific quality profiles.
	class CompressionProfile
	{
	public:
		std::function<void(bc6h_enc_settings*)> GetBC6() const
		{
			return bc6;
		}

		std::function<void(bc7_enc_settings*)> GetBC7(bool discardAlpha) const
		{
			if (discardAlpha)
			{
				return bc7;
			}

			return bc7Alpha;
		}

		std::function<void(astc_enc_settings*, int block_width, int block_height)> GetASTC(bool discardAlpha) const
		{
			if (discardAlpha)
			{
				return astc;
			}

			return astcAlpha;
		}

		std::function<void(bc6h_enc_settings*)>   bc6;
		std::function<void(bc7_enc_settings*)>    bc7;
		std::function<void(bc7_enc_settings*)>    bc7Alpha;
		std::function<void(astc_enc_settings*, int block_width, int block_height)>   astc;
		std::function<void(astc_enc_settings*, int block_width, int block_height)>   astcAlpha;
	};

	CMImageEncoder::CMImageEncoder()
	{

	}

	CMImageEncoder::~CMImageEncoder()
	{

	}

	static bool IsFormatSupportedForBCn(CMImageFormat pixelFormat, u32 bitDepth, CMImageSourceFormat outputFormat)
	{
		switch (pixelFormat)
		{
		case CMImageFormat::R8:
			if (outputFormat == CMImageSourceFormat::BC4 && bitDepth == 8)
				return true;
			break;
		case CMImageFormat::RG8:
			if (outputFormat == CMImageSourceFormat::BC5 && bitDepth == 8)
				return true;
			break;
		case CMImageFormat::RGB8:
			if ((outputFormat == CMImageSourceFormat::BC1 || outputFormat == CMImageSourceFormat::BC7) && bitDepth == 8)
				return true;
			break;
		case CMImageFormat::RGBA8:
			if ((outputFormat == CMImageSourceFormat::BC3 || outputFormat == CMImageSourceFormat::BC7) && bitDepth == 8)
				return true;
			break;
		case CMImageFormat::RGB16:
		case CMImageFormat::RGBA16:
			if (outputFormat == CMImageSourceFormat::BC6H)
				return true;
			break;
		}

		return false;
	}

	u64 CMImageEncoder::GetCompressedSizeRequirement(const CAImage& surface, CMImageSourceFormat destFormat)
	{
		u64 destSize = 0;
		switch (destFormat)
		{
		case CMImageSourceFormat::BC1:
			destSize = surface.width * surface.height / 2;
			break;
		case CMImageSourceFormat::BC3:
			destSize = surface.width * surface.height;
			break;
		case CMImageSourceFormat::BC4:
			destSize = surface.width * surface.height / 2;
			break;
		case CMImageSourceFormat::BC5:
			destSize = surface.width * surface.height;
			break;
		case CMImageSourceFormat::BC6H:
			destSize = surface.width * surface.height;
			break;
		case CMImageSourceFormat::BC7:
			destSize = surface.width * surface.height;
			break;
		default:
			return destSize; // Should never happen
		}

		return destSize;
	}

	bool CMImageEncoder::EncodeToBCn(const CAImage& image, void* outData, CMImageSourceFormat destFormat, Quality quality)
	{
#if PLATFORM_DESKTOP
		errorMessage = "";

		if (!image.IsValid())
			return false;

		if ((int)destFormat < (int)CMImageSourceFormat::BC1 ||
			(int)destFormat > (int)CMImageSourceFormat::BC7)
			return false;

		using QualityPair = Pair<Quality, CompressionProfile>;

		static const QualityPair qualityMap[4] = {
			QualityPair{ Quality_Preview, CompressionProfile{
				GetProfile_bc6h_veryfast, GetProfile_ultrafast, GetProfile_alpha_ultrafast, GetProfile_astc_fast, GetProfile_astc_alpha_fast } },
			QualityPair{ Quality_Fast, CompressionProfile{
				GetProfile_bc6h_fast, GetProfile_fast, GetProfile_alpha_fast, GetProfile_astc_fast, GetProfile_astc_alpha_fast } },
			QualityPair{ Quality_Normal, CompressionProfile{
				GetProfile_bc6h_basic, GetProfile_basic, GetProfile_alpha_basic, GetProfile_astc_alpha_slow, GetProfile_astc_alpha_slow } },
			QualityPair{ Quality_Slow, CompressionProfile{
				GetProfile_bc6h_basic, GetProfile_basic, GetProfile_alpha_basic, GetProfile_astc_alpha_slow, GetProfile_astc_alpha_slow } }
		};
		static const CompressionProfile defaultQuality = { GetProfile_bc6h_basic, GetProfile_basic, GetProfile_alpha_basic, GetProfile_astc_alpha_slow, GetProfile_astc_alpha_slow };

		if (!IsFormatSupportedForBCn(image.GetFormat(), image.GetBitDepth(), destFormat))
		{
			errorMessage = "DestFormat not supported";
			return false;
		}
		if (image.GetWidth() % 4 > 0 || image.GetHeight() % 4 > 0)
		{
			errorMessage = "Width or height not divisible by 4";
			return false;
		}

		u32 pixelStride = image.GetBitsPerPixel() / 8;
		if (image.GetNumChannels() == 3) // Input image is always in either layouts: 1, 2 or 4 channels
			pixelStride = image.GetBitsPerPixel() * 4 / 3 / 8;
		
		rgba_surface surface{};
		surface.ptr = (u8*)image.GetDataPtr();
		surface.width = image.GetWidth();
		surface.height = image.GetHeight();
		surface.stride = surface.width * pixelStride;

		u32 destSize = 0;
		bc6h_enc_settings bc6Settings{};
		bc7_enc_settings bc7Settings{};
		bool useAlpha = false;

		if (image.format == CMImageFormat::RGBA8 ||
			image.format == CMImageFormat::RGBA16 ||
			image.format == CMImageFormat::RGBA32)
			useAlpha = true;
		
		switch (destFormat)
		{
		case CMImageSourceFormat::BC1:
			destSize = surface.width * surface.height / 2;
			CompressBlocksBC1(&surface, (uint8_t*)outData);
			break;
		case CMImageSourceFormat::BC3:
			destSize = surface.width * surface.height;
			CompressBlocksBC3(&surface, (uint8_t*)outData);
			break;
		case CMImageSourceFormat::BC4:
			destSize = surface.width * surface.height / 2;
			CompressBlocksBC4(&surface, (uint8_t*)outData);
			break;
		case CMImageSourceFormat::BC5:
			destSize = surface.width * surface.height;
			CompressBlocksBC5(&surface, (uint8_t*)outData);
			break;
		case CMImageSourceFormat::BC6H:
			destSize = surface.width * surface.height;
			qualityMap[quality].second.GetBC6()(&bc6Settings);
			CompressBlocksBC6H(&surface, (uint8_t*)outData, &bc6Settings);
			break;
		case CMImageSourceFormat::BC7:
			destSize = surface.width * surface.height;
			qualityMap[quality].second.GetBC7(!useAlpha)(&bc7Settings);
			CompressBlocksBC7(&surface, (uint8_t*)outData, &bc7Settings);
			break;
		default:
			return false; // Should never happen
		}

		return true;
#else // !PLATFORM_DESKTOP
		errorMessage = "Unsupported Platform";
		return false;
#endif
	}

} // namespace CE
