#pragma once

namespace CE
{
	using FUIIndex = u32;

	enum class FUIShaderType : u32
	{
		SolidColor = 0,
		Texture,
		LinearGradient,
		RadialGradient,
		ConicGradient,
		SDFText,
		Custom
	};
	ENUM_CLASS(FUIShaderType);

	enum class FUIDrawItemFlags : u32
	{
		None = 0,
		TextureTileX = 1 << 0,
		TextureTileY = 1 << 1,
		ImageFitCover = 1 << 2,
		ImageFitContain = 1 << 3,
	};
	ENUM_CLASS_FLAGS(FUIDrawItemFlags);

	ENUM()
	enum class FUIBlendMode : u32
	{
		Normal = 0, // Standard alpha-over
		Additive,
		Multiply
	};
	ENUM_CLASS(FUIBlendMode);

	struct FUIVertex
	{
		Vec2 pos;
		Vec2 uv;
		u32 color = 0;
		u32 drawItemIndex = 0;
	};

	struct alignas(16) FUIClipRect
	{
		Matrix4x4 clipInverseTransform;
		Vec4 cornerRadii;
		Vec2 clipHalfSize;
	};

	struct FUIGradientStop
	{
		u32 packedColor = 0;
		f32 position = 0;
	};

	struct FUIDrawItem
	{
		FUIShaderType shaderType = FUIShaderType::SolidColor;
		u32 textureIndex0 = 0;
		u32 textureIndex1 = 0;
		FUIDrawItemFlags drawItemFlags = FUIDrawItemFlags::None;

		int clipRectIndex = -1;
		int gradientStartIndex = 0;
		int gradientStopCount = 0;
		u32 userFlags = 0;

		// 128 bytes: per-shader payload
		f32 data[32] = {};
	};
	static_assert(sizeof(FUIDrawItem) == 160);

	struct FUIDrawCmd
	{
		u32 indexOffset = 0;
		u32 indexCount = 0;
		u32 vertexOffset = 0;
		FUIBlendMode blendMode = FUIBlendMode::Normal;
		u32 customShaderId = 0;
		Rect scissorRect;
	};

}

#include "FUIDrawData.rtti.h"