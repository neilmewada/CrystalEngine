#ifndef __VIEW_DATA_HLSL__
#define __VIEW_DATA_HLSL__

cbuffer _PerViewData : SRG_PerView(b0)
{
    float4x4 _ViewMatrix;
    float4x4 _ViewProjectionMatrix;
    float4x4 _ProjectionMatrix;
    float4 _ViewPosition;
    float2 _PixelResolution;
    float _FarPlane;
    float _NearPlane;
};

inline float LinearizeDepth(float depth)
{
    return _NearPlane * _FarPlane / (_FarPlane + depth * (_NearPlane - _FarPlane));
}

#endif // __VIEW_DATA_HLSL__
