#pragma once

namespace CE
{
	/// Texture pixel format
	ENUM()
	enum class TextureFormat
	{
		None = 0,
		RGBA8,
		BGRA8,
		RGB8,
		BGR8,

		RGB565,

		R8,
		// Single channel 16-bit integer
		R16,
		RG8,
		RG16,

		// Single channel 16-bit float
		RHalf,
		RGHalf,
		RGBAHalf,

		// Single channel 32-bit float
		RFloat,
		RGFloat,
		RGBAFloat,

		BC1,
		BC3,
		BC4,
		BC5,
		BC6H,
		BC7
	};
	ENUM_CLASS_FLAGS(CE::TextureFormat);

	ENGINE_API RHI::Format ToRHIFormat(CE::TextureFormat format, bool isSrgb = false);

	ENGINE_API CE::TextureFormat ToTextureFormat(CMImageFormat format);

	/// Texture source data format
	ENUM()
	enum class TextureSourceCompressionFormat
	{
		None = 0,
		PNG,
		HDR, EXR,
		JPG,
		BC1,
		BC3,
		BC4,
		BC5,
		BC6H,
		BC7,
	};
	ENUM_CLASS_FLAGS(TextureSourceCompressionFormat);

	ENGINE_API bool TextureSourceCompressionFormatIsBCn(TextureSourceCompressionFormat sourceCompressionFormat);

	ENUM()
	enum class TextureCompressionQuality
	{
		//! @brief No compression. Uses highest memory footprint.
		None = 0,
		Normal,
		High,
		Low,
		Default = Normal
	};
	ENUM_CLASS(TextureCompressionQuality);

	ENUM()
	enum class TextureCompressionSettings : int
	{
		ECONST(DisplayName = "Default (Color BC1/3)")
		Default, // Color BC1/3
		ECONST(DisplayName = "Normal Map (BC1/5)")
		NormalMap, // BC1/5: BC1 = low quality , BC5 = high quality
		ECONST(DisplayName = "Grayscale (R8/16 or BC4)")
		Grayscale, // R8/16 or BC4
		ECONST(DisplayName = "Grayscale Alpha (RG16/32 or BC5)")
		GrayscaleAlpha, // RG16/32 or BC5
		ECONST(DisplayName = "HDR (BC6H)")
		HDR, // BC6H
		ECONST(DisplayName = "HDR Uncompressed (RGBA16 Half)")
		HDRUncompressed, // RGBAHalf
		ECONST(DisplayName = "BC7 RGBA")
		BC7, // BC7 RGBA
		ECONST(DisplayName = "Color Uncompressed RGB(A)")
		ColorUncompressed,
	};
	ENUM_CLASS(TextureCompressionSettings);

	ENUM()
	enum class TextureFilter : int
	{
		Linear = 0,
		Nearest,
		Cubic
	};
	ENUM_CLASS(TextureFilter);

	ENUM()
	enum class TextureAddressMode : int
	{
		Repeat = 0,
		Clamp,
		ClampToBorder
	};
	ENUM_CLASS(TextureAddressMode);

	ENGINE_API RHI::SamplerAddressMode TextureAddressModeToSamplerMode(TextureAddressMode mode);

	ENUM()
	enum class TextureColorSpace : int
	{
		None = 0,
		ECONST(DisplayName = "sRGB")
		SRGB
	};
	ENUM_CLASS(TextureColorSpace);

	ENUM()
	enum class TextureDimension
	{
		None = 0,
		Tex2D,
		Tex3D,
		TexCube,
		Tex2DArray,
		TexCubeArray
	};
	ENUM_CLASS(TextureDimension);

	ENUM()
	enum MSAA
	{
		ECONST(DisplayName = "x1")
		MSAA1,
		ECONST(DisplayName = "x2")
		MSAA2,
		ECONST(DisplayName = "x4")
		MSAA4,
		ECONST(DisplayName = "x8")
		MSAA8
	};

} // namespace CE

#include "TextureDefines.rtti.h"
