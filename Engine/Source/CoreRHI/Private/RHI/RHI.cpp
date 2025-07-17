
#include "RHI/Resources.h"
#include "RHI/RHI.h"

namespace CE::RHI
{
	CORERHI_API DynamicRHI* gDynamicRHI = nullptr;

	struct FormatEntry
	{
		RHI::Format format;
		u32 numChannels;
		u32 bitsPerPixel;
	};

	constexpr FormatEntry formatEntries[] = {
		{ RHI::Format::R8_UNORM,			1, 8 },
		{ RHI::Format::R8_SNORM,			1, 8 },
		{ RHI::Format::R8_SRGB,				1, 8 },
		{ RHI::Format::R8G8_UNORM,			2, 16 },
		{ RHI::Format::R8G8_SRGB,			2, 16 },
		{ RHI::Format::R8G8B8A8_SRGB,		4, 32 },
		{ RHI::Format::R8G8B8A8_UNORM,		4, 32 },
		{ RHI::Format::R8G8B8A8_SNORM,		4, 32 },
		{ RHI::Format::B8G8R8A8_SRGB,		4, 32 },
		{ RHI::Format::B8G8R8A8_UNORM,		4, 32 },
		{ RHI::Format::B8G8R8A8_SNORM,		4, 32 },
		{ RHI::Format::R8G8B8_UNORM,		3, 24 },
		{ RHI::Format::R8G8B8_SNORM,		3, 24 },
		{ RHI::Format::R8G8B8_SRGB,			3, 24 },
		{ RHI::Format::B8G8R8_UNORM,		3, 24 },
		{ RHI::Format::B8G8R8_SNORM,		3, 24 },
		{ RHI::Format::B8G8R8_SRGB,			3, 24 },
		{ RHI::Format::R5G6B5_UNORM,		3, 16 },
		{ RHI::Format::B5G6R5_UNORM,		3, 16 },
		{ RHI::Format::R16_UNORM,			1, 16 },
		{ RHI::Format::R16_SNORM,			1, 16 },
		{ RHI::Format::R16_SFLOAT,			1, 16 },
		{ RHI::Format::R16G16_UNORM,		2, 32 },
		{ RHI::Format::R16G16_SNORM,		2, 32 },
		{ RHI::Format::R16G16_UINT,			2, 32 },
		{ RHI::Format::R16G16_SINT,			2, 32 },
		{ RHI::Format::R16G16_SFLOAT,		2, 32 },
		{ RHI::Format::R16G16B16A16_SFLOAT, 4, 64 },
		{ RHI::Format::R32_UINT,			1, 32 },
		{ RHI::Format::R32_SINT,			1, 32 },
		{ RHI::Format::R32_SFLOAT,			1, 32 },
		{ RHI::Format::R32G32_UINT,			2, 64 },
		{ RHI::Format::R32G32_SINT,			2, 64 },
		{ RHI::Format::R32G32_SFLOAT,		2, 64 },
		{ RHI::Format::R32G32B32A32_SFLOAT,	4, 128 },
		{ RHI::Format::D16_UNORM_S8_UINT,	2, 24 },
		{ RHI::Format::D16_UNORM,			1, 16 },
		{ RHI::Format::D24_UNORM_S8_UINT,	2, 32 },
		{ RHI::Format::D32_SFLOAT_S8_UINT,	2, 40 },
		{ RHI::Format::D32_SFLOAT,			1, 32 },
		{ RHI::Format::BC1_RGB_UNORM,		3, 4 },
		{ RHI::Format::BC1_RGB_SRGB,		3, 4 },
		{ RHI::Format::BC1_RGBA_UNORM,		4, 4 },
		{ RHI::Format::BC1_RGBA_SRGB,		4, 4 },
		{ RHI::Format::BC3_UNORM,			4, 8 },
		{ RHI::Format::BC3_SRGB,			4, 8 },
		{ RHI::Format::BC4_UNORM,			1, 4 },
		{ RHI::Format::BC5_UNORM,			2, 8 },
		{ RHI::Format::BC7_UNORM,			4, 8 },
		{ RHI::Format::BC7_SRGB,			4, 8 },
		{ RHI::Format::BC6H_UFLOAT,			3, 8 },
		{ RHI::Format::BC6H_SFLOAT,			3, 8 }
	};

	CORERHI_API u32 GetBitsPerPixelForFormat(RHI::Format format)
	{
		for (int i = 0; i < COUNTOF(formatEntries); i++)
		{
			if (formatEntries[i].format == format)
			{
				return formatEntries[i].bitsPerPixel;
			}
		}

		return 0;
	}

	u32 GetBitDepthForFormat(RHI::Format format)
	{
		for (int i = 0; i < COUNTOF(formatEntries); i++)
		{
			if (formatEntries[i].format == format)
			{
				return formatEntries[i].bitsPerPixel / formatEntries[i].numChannels;
			}
		}

		return 0;
	}

	CORERHI_API u32 GetNumChannelsForFormat(RHI::Format format)
	{
		for (int i = 0; i < COUNTOF(formatEntries); i++)
		{
			if (formatEntries[i].format == format)
			{
				return formatEntries[i].numChannels;
			}
		}

		return 0;
	}

	CORERHI_API SIZE_T GetVertexInputTypeSize(VertexInputAttribute input)
	{
		switch (input) {
		case VertexInputAttribute::Position:
			return sizeof(Vec3);
		case VertexInputAttribute::UV:
			return sizeof(Vec2);
		case VertexInputAttribute::Normal:
		case VertexInputAttribute::Binormal:
		case VertexInputAttribute::Tangent:
			return sizeof(Vec3);
		case VertexInputAttribute::Color:
			return sizeof(Vec4);
		default:
			return 0;
		}

		return 0;
	}

	ShaderSemantic ShaderSemantic::Parse(const String& string)
	{
		ShaderSemantic result{};
		result.index = 0;

		if (string.GetLength() < 2)
			return result;
		
		String name{};
		name.Reserve(string.GetLength());
		String numberString{};
		
		for (int i = 0; i < string.GetLength(); i++)
		{
			if (String::IsNumeric(string[i]))
			{
				numberString.Append(string[i]);
			}
			else if (String::IsAlphabet(string[i]) && numberString.IsEmpty())
			{
				name.Append(string[i]);
			}
		}

		String nameUpper = name.ToUpper();
		EnumType* enumType = GetStaticEnum<VertexInputAttribute>();
		if (!enumType)
			return result;

		for (int i = 0; i < enumType->GetConstantsCount(); i++)
		{
			String enumName = enumType->GetConstant(i)->GetName().GetString().ToUpper();
			String alternativeEnumName = "";
			if (enumName == "UV")
			{
				alternativeEnumName = "TEXCOORD";
			}
			if (enumName == nameUpper || alternativeEnumName == nameUpper)
			{
				result.attribute = (VertexInputAttribute)enumType->GetConstant(i)->GetValue();
				break;
			}
		}

		if (!numberString.IsEmpty())
		{
			String::TryParse(numberString, result.index);
		}

		return result;
	}

	String ShaderSemantic::ToString() const
	{
		String result{};

		EnumType* enumType = GetStaticEnum<VertexInputAttribute>();
		if (!enumType)
			return result;

		EnumConstant* enumConst = enumType->FindConstantWithValue((s64)attribute);
		if (!enumConst)
			return result;

		result = enumConst->GetName().GetString();

		if (index > 0)
			result.Concatenate((s64)index);

		return result;
	}

	SIZE_T CE::RHI::SamplerDescriptor::GetHash() const
	{
		SIZE_T hash = CE::GetHash(addressModeU);
		CombineHash(hash, addressModeV);
		CombineHash(hash, addressModeW);
		CombineHash(hash, samplerFilterMode);
		CombineHash(hash, borderColor);
		CombineHash(hash, enableAnisotropy);
		if (enableAnisotropy)
		{
			CombineHash(hash, maxAnisotropy);
		}

		return hash;
	}

}
