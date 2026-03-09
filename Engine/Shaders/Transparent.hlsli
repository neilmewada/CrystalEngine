#ifndef __TRANSPARENT_HLSL__
#define __TRANSPARENT_HLSL__

#include "StandardIncludes.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD0;
    INSTANCING()
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
    float3 tangent : TEXCOORD3;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput output;
    output.position = LOCAL_TO_CLIP_SPACE(float4(input.position, 1.0), input);
    output.worldPos = LOCAL_TO_WORLD_SPACE(float4(input.position, 1.0), input).xyz;
    output.normal = LOCAL_TO_WORLD_SPACE(float4(input.normal, 0), input).xyz;
    output.tangent = LOCAL_TO_WORLD_SPACE(float4(input.tangent.xyz, 0), input).xyz;
    //output.bitangent = -cross(output.normal, output.tangent);
    output.uv = input.uv;
    return output;
}
#endif

#if FRAGMENT

cbuffer _Material : SRG_PerMaterial(b0)
{
    float4 _Albedo;
    float  _Metallic;
    float  _Roughness;
    float  _NormalStrength;
    float  _AmbientOcclusion;
};

Texture2D<float4> _AlbedoTex : SRG_PerMaterial(t1);
Texture2D<float> _RoughnessTex : SRG_PerMaterial(t2);
Texture2D<float4> _NormalTex : SRG_PerMaterial(t3);
Texture2D<float> _MetallicTex : SRG_PerMaterial(t4);

SamplerState _AlbedoTexSampler : SRG_PerMaterial(t5);

float4 FragMain(PSInput input) : SV_TARGET
{
    return _Albedo;
}

#endif


#endif
