#ifndef __SCENE_DATA_HLSL__
#define __SCENE_DATA_HLSL__

#include "LightingData.hlsli"

#if FRAGMENT || COMPUTE

cbuffer _DirectionalLightsArray : SRG_PerScene(b0)
{
    DirectionalLight _DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
};

TextureCube<float4> _Skybox : SRG_PerScene(t1);
SamplerState _DefaultSampler : SRG_PerScene(s2);
TextureCube<float4> _SkyboxIrradiance : SRG_PerScene(t3);
SamplerState _SkyboxSampler : SRG_PerScene(s4);
Texture2D<float2> _BrdfLut : SRG_PerScene(s5);

SamplerState _ShadowMapSampler : SRG_PerScene(s6);

#endif // FRAGMENT

StructuredBuffer<LocalLightData> _LocalLights : SRG_PerScene(t7);

cbuffer _LightData : SRG_PerScene(b8)
{
    float4 ambient;
    uint totalDirectionalLights;
    uint totalLocalLights; // total point + spot lights
    uint tileSizeX; // e.g., 16
    uint tileSizeY; // e.g., 16
    uint lightsPerTile;
    uint lightIndexPoolCapacity;    // max uints in _LightIndexPool
};

cbuffer _SceneData : SRG_PerScene(b11)
{
    float _TimeElapsed;
};

#endif
