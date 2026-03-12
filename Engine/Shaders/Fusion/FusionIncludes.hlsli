#ifndef __FUSION_INCLUDES_HLSLI__
#define __FUSION_INCLUDES_HLSLI__

#include "Core/Macros.hlsli"
#include "Core/ViewData.hlsli"
#include "Core/Gamma.hlsli"

Texture2D<float4> _Textures[] : SRG_PerScene(t0);
SamplerState _Samplers[] : SRG_PerScene(s1);

#endif