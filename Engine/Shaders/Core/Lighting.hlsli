#ifndef __LIGHTING_HLSL__
#define __LIGHTING_HLSL__

#include "PBR/BRDF.hlsli"
#include "LightingData.hlsli"

#if FRAGMENT

// - Per Pass -

Texture2D<float> _DirectionalShadowMap : SRG_PerPass(t0);

// index into _LocalLights
StructuredBuffer<uint> _LightIndexPool : SRG_PerPass(t1);

// {offset, count}
StructuredBuffer<uint2> _TileHeaders : SRG_PerPass(t2);

Texture2DMS<float> _DepthMap : SRG_PerPass(t3);

inline float sqr(float x)
{
    return x * x;
}

float AttenuateCusp(float distance, float radius, float max_intensity, float falloff)
{
    float s = distance / radius;

    if (s >= 1.0)
        return 0.0;

    float s2 = sqr(s);

    return max_intensity * sqr(1 - s2) / (1 + falloff * s);
}

float CalculateDirectionalShadow(in float4 lightSpacePos, in float NdotL)
{
    float3 projectionCoords = lightSpacePos.xyz / lightSpacePos.w;
    projectionCoords = projectionCoords * float3(0.5, 0.5, 1.0) + float3(0.5, 0.5, 0); // In Vulkan, Z axis is already in [0, 1] range, so we ONLY map X and Y to [0, 1] range FROM [-1, 1] range
    projectionCoords.z = clamp(projectionCoords.z, 0, 1);

    float currentDepth = projectionCoords.z;
    float bias = 0.005;
    bias = max(0.01 * (1.0 - NdotL), 0.005);
    
    float w; float h;
    _DirectionalShadowMap.GetDimensions(w, h);
    float2 texelSize = float2(1, 1) / float2(w, h);
    float shadow = 0;
    for (int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcfDepth = _DirectionalShadowMap.Sample(_ShadowMapSampler, projectionCoords.xy + float2(x, y) * texelSize);
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

float3 CalculateDiffuseIrradiance(MaterialInput material, float3 N, float3 V)
{
    float NdotV = clamp(dot(N, V), 0, 1);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, material.albedo.rgb, material.metallic);
    
    float3 kS = FresnelSchlickRoughness(NdotV, F0, material.roughness);
    float3 kD = float3(1, 1, 1) - kS;
    kD *= (1.0 - material.metallic);
    float3 irradiance = _SkyboxIrradiance.Sample(_SkyboxSampler, N).rgb;
    float3 diffuse = irradiance * material.albedo;
    float3 ambient = (kD * diffuse) * material.ambient;
    
    return ambient;
}

float3 CalculateSpecularIBL(MaterialInput material, float3 N, float3 V, out float3 F)
{
    float NdotV = clamp(dot(N, V), 0, 1);
    float roughness = clamp(material.roughness, 0, 1);
    float3 R = 2 * dot( V, N ) * N - V;
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, material.albedo.rgb, material.metallic);

    uint w; uint h;
    _Skybox.GetDimensions(w, h);
    float maxMipLevel = log2(min(w, h)) - 1;
    //maxMipLevel = 9;

    F = FresnelSchlickRoughness(NdotV, F0, roughness);
    float3 prefilteredColor = _Skybox.SampleLevel(_SkyboxSampler, R, roughness * maxMipLevel).rgb; // 10 max Mip levels
    float2 envBrdf = _BrdfLut.Sample(_DefaultSampler, float2(NdotV, roughness));

    return prefilteredColor * (F * envBrdf.x + envBrdf.y);
}

float3 ComputeSkyboxIBL(MaterialInput material, float3 N, float3 V)
{
    float3 F = float3(0, 0, 0);
    float3 diffuse = CalculateDiffuseIrradiance(material, N, V).rgb;
    float3 specular = CalculateSpecularIBL(material, N, V, F);

    float3 kS = F;
    float3 kD = float3(1, 1, 1) - kS;
    kD *= (1.0 - material.metallic);

    return (kD * diffuse + specular) * material.ambient;
}

#endif // FRAGMENT

#endif // __LIGHTING_HLSL__
