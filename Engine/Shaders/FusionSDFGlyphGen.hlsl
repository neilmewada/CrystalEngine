
#include "Core/Macros.hlsli"
#include "Core/Gamma.hlsli"

struct VSInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = float4(input.position.xy, 0.0, 1.0);
    o.uv = input.uv;
    return o;
}

#endif

#if FRAGMENT

Texture2D<float> _FontAtlas : SRG_PerMaterial(t0);
SamplerState _FontAtlasSampler : SRG_PerMaterial(s1);

cbuffer _MaterialData : SRG_PerMaterial(b2)
{
    int _Spread;
};

inline uint GetPixelState(float2 uv, float w, float h)
{
    float sample = _FontAtlas.Sample(_FontAtlasSampler, uv).r;
    return sample > 0.1 ? 1 : 0;
}

float4 FragMain(PSInput input) : SV_TARGET
{
    uint w; uint h;
    _FontAtlas.GetDimensions(w, h);

    const int spread = _Spread; // 7;
    const float stepSize = 0.1;
    uint basePixelState = GetPixelState(input.uv, w, h);
    float maxPossibleSqrDistance = spread * spread + spread * spread;
    float minSqrDistance = maxPossibleSqrDistance;

    for (float x = -spread; x <= spread; x += stepSize)
    {
        for (float y = -spread; y <= spread; y += stepSize)
        {
            float2 uv = input.uv + float2(float(x) / w, float(y) / h);
            uint curPixelState = GetPixelState(uv, w, h);
            float sqrDistance = x * x + y * y;

            if (basePixelState != curPixelState)
            {
                minSqrDistance = min(sqrDistance, minSqrDistance);
            }
        }
    }

    float ratio = sqrt(minSqrDistance) / sqrt(maxPossibleSqrDistance) * (basePixelState == 1 ? 1.0 : -1.0);

    return float4((ratio + 1) * 0.5, 0, 0, 1);
}

#endif