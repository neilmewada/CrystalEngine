#ifndef __VIEW_DATA_HLSL__
#define __VIEW_DATA_HLSL__

cbuffer _PerViewData : SRG_PerView(b0)
{
    float4x4 viewMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 projectionMatrix;
    float4 viewPosition;
    float2 pixelResolution;
    float farPlane;
    float nearPlane;
};

inline float LinearizeDepth(float depth)
{
    return nearPlane * farPlane / (farPlane + depth * (nearPlane - farPlane));
}

#endif // __VIEW_DATA_HLSL__
