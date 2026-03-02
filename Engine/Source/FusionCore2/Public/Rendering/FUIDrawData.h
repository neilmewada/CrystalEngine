#pragma once

namespace CE
{
	using FUIIndex = u16;

	enum class FUIShaderType : u32
	{
		SolidColor = 0,
		Texture,
		Gradient,
		SDFText
	};
	ENUM_CLASS(FUIShaderType);

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
		u32 flags = 0;

		int clipRectIndex = -1;
		u32 _pad[3];

		// 128 bytes: per-shader payload
		Vec4 data[8] = {};
	};

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