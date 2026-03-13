
#include "FusionIncludes.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD1;
    float4 color : TEXCOORD2;
    nointerpolation uint drawItemIndex : TEXCOORD3;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = TRANSFORM_POSITION(input.position);
    o.uv = input.uv;
    o.color = input.color;
    o.drawItemIndex = input.drawItemIndex;
    return o;
}

#endif

#if FRAGMENT

float4 FragMain(PSInput input) : SV_TARGET
{
    return input.color;
}

#endif
