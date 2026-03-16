#ifndef __FUSION_INCLUDES_HLSLI__
#define __FUSION_INCLUDES_HLSLI__

#include "Core/Macros.hlsli"
#include "Core/ViewData.hlsli"
#include "Core/Gamma.hlsli"

#define TRANSFORM_POSITION(pos2f) mul(mul(float4(pos2f, 0.0, 1.0), _LayerTransform), _ViewProjectionMatrix)

#if FRAGMENT

SamplerState _Samplers[16] : SRG_PerScene(s0);
Texture2D<float4> _Textures[] : SRG_PerScene(t1);

#endif

cbuffer _LayerData : SRG_PerSubPass(b0)
{
    float4x4 _LayerTransform;
};

enum class FUIShaderType : uint
{
	SolidColor = 0,
	Texture,
	LinearGradient,
	RadialGradient,
	ConicGradient,
	SDFText,
	Custom
};

enum class FUIDrawItemFlags : uint
{
	None = 0,
	TextureTileX = 1 << 0,
	TextureTileY = 1 << 1,
	ImageFitCover = 1 << 2,
	ImageFitContain = 1 << 3,
};

struct FUIDrawItem
{
	FUIShaderType shaderType;
	uint textureIndex;
	uint samplerIndex;
	FUIDrawItemFlags drawItemFlags;

	int clipRectIndex;
	int gradientStartIndex;
	int gradientStopCount;
	uint userFlags;

	// 128 bytes: per-shader payload
	float data[32];
};

// StructuredBuffer<FUIDrawItem> _DrawItems : SRG_PerObject(t0);

struct VSInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    uint   drawItemIndex : TEXCOORD1;
};

#endif