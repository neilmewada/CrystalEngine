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
    float3 bitangent : TEXCOORD4;
    float4 clipPos : TEXCOORD5;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput output;
    output.position = LOCAL_TO_CLIP_SPACE(float4(input.position, 1.0), input);
    output.worldPos = LOCAL_TO_WORLD_SPACE(float4(input.position, 1.0), input).xyz;
    output.normal = LOCAL_TO_WORLD_SPACE(float4(input.normal, 0), input).xyz;
    output.tangent = LOCAL_TO_WORLD_SPACE(float4(input.tangent.xyz, 0), input).xyz;
    output.bitangent = -cross(output.normal, output.tangent);
    output.uv = input.uv;
    output.clipPos = output.position;
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
    float3 diffuse = 0;
    float3 specular = 0;
    float3 vertNormal = normalize(input.normal);
    float3 viewDir = normalize(viewPosition - input.worldPos);
    float3 tangent = normalize(input.tangent);
    float3 bitangent = normalize(input.bitangent);

    float4 normalMapSample = _NormalTex.Sample(_AlbedoTexSampler, input.uv);
    float3 tangentSpaceNormal = normalize(normalMapSample.xyz * 2.0 - 1.0);

    float3x3 tangentToWorld = float3x3(tangent, bitangent, vertNormal);
    float3 normal = normalize(mul(tangentSpaceNormal, tangentToWorld));

    float4 albedoSample = _AlbedoTex.Sample(_AlbedoTexSampler, input.uv);

    MaterialInput material;
    material.albedo = GammaToLinear(_Albedo.rgb * albedoSample.rgb);
    material.metallic = _Metallic * _MetallicTex.Sample(_AlbedoTexSampler, input.uv);
    material.roughness = _Roughness * _RoughnessTex.Sample(_AlbedoTexSampler, input.uv);
    material.ambient = _AmbientOcclusion;

    float NdotV = clamp(dot(normal, viewDir), 0.0, 1.0);
    float3 F0 = float3(0.04, 0.04, 0.04);
    float fresnel = FresnelSchlickRoughness(NdotV, F0, material.roughness).x;
    float alpha = lerp(_Albedo.a * albedoSample.a, 1.0, fresnel);

    float3 Lo = float3(0, 0, 0);

    uint i = 0;
    for (i = 0; i < totalDirectionalLights; i++)
    {
        LightInput light;
        light.lightRadiance = _DirectionalLights[i].colorAndIntensity.rgb * _DirectionalLights[i].colorAndIntensity.a;
        light.lightDir = -_DirectionalLights[i].direction;
        light.normal = normal;
        light.viewDir = viewDir;
        light.halfway = normalize(viewDir + light.lightDir);

        float shadow = 0.0;
        if (_DirectionalLights[i].shadow > 0)
        {
            float4 lightSpacePos = mul(float4(input.worldPos, 1.0), _DirectionalLights[i].lightSpaceMatrix);
            shadow = CalculateDirectionalShadow(lightSpacePos, dot(vertNormal, light.lightDir));
        }
        shadow = clamp(shadow, 0, 1);

        Lo += CalculateBRDF(light, material) * (1.0 - shadow);
    }

    const float2 screenPos = input.position.xy;

    const uint width = (uint)pixelResolution.x;
    const uint height = (uint)pixelResolution.y;
    const uint tilesX = (width + tileSizeX - 1) / tileSizeX;
    const uint tilesY = (height + tileSizeY - 1) / tileSizeY;

    const uint tx = (uint)(screenPos.x / tileSizeX);
    const uint ty = (uint)(screenPos.y / tileSizeY);
    const uint tileId = ty * tilesX + tx;

    const uint localLightBase = _TileHeaders[tileId].x;
    const uint numLocalLights = _TileHeaders[tileId].y;

    for (i = 0; i < numLocalLights; i++)
    {
        uint lightIndex = _LightIndexPool[localLightBase + i];
        LocalLightType lightType = _LocalLights[lightIndex].type;

        LightInput light;
        float3 luminosity = _LocalLights[lightIndex].colorAndIntensity.rgb * _LocalLights[lightIndex].colorAndIntensity.a;

        if (lightType == LocalLightType_Point)
        {
            light.lightDir = _LocalLights[lightIndex].worldPosAndRange.xyz - input.worldPos;
            float distance = length(light.lightDir);
            float attenuation = AttenuateCusp(distance, _LocalLights[lightIndex].worldPosAndRange.w, 2.0, 4.0);
            light.lightDir = normalize(light.lightDir);
            light.lightRadiance = luminosity * attenuation;
        }
        else
        {
            continue;
        }

        light.normal = normal;
        light.viewDir = viewDir;
        light.halfway = normalize(viewDir + light.lightDir);

        float shadow = 0.0;

        Lo += CalculateBRDF(light, material) * (1.0 - shadow);
    }

    float3 color = ComputeSkyboxIBL(material, normal, viewDir);

    color += Lo;
    color = color / (color + float3(1.0, 1.0, 1.0) * 0.5); // HDR Tonemapping

    color = LinearToGamma(color); // Convert to Gamma space

    return float4(color, alpha);
}

#endif


#endif // __TRANSPARENT_HLSL__
