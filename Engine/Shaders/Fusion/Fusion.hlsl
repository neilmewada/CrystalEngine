
#include "FusionIncludes.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD1;
    float4 color : TEXCOORD2;
    nointerpolation uint drawItemIndex : TEXCOORD3;
    float2 layerPos : TEXCOORD4;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = TRANSFORM_POSITION(input.position);
    o.uv = input.uv;
    o.color = input.color;
    o.drawItemIndex = input.drawItemIndex;
    o.layerPos = input.position;
    return o;
}

#endif

#if FRAGMENT

float SDFRoundedRect(float2 p, float2 halfSize, float4 radii)
{
    float r = p.x > 0.0
        ? (p.y > 0.0 ? radii.z : radii.y)
        : (p.y > 0.0 ? radii.w : radii.x);
    float2 q = abs(p) - halfSize + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

float4 FragMain(PSInput input) : SV_TARGET
{
    FUIDrawItem item = _DrawItems[input.drawItemIndex];
    float4 color = input.color;
    float2 uv = input.uv;

    if (item.clipRectIndex >= 0)
    {
        FUIClipRect clip = _ClipRects[item.clipRectIndex];
        float2 clipLocalPos = mul(float4(input.layerPos, 0.0, 1.0), clip.clipInverseTransform).xy;
        float dist = SDFRoundedRect(clipLocalPos, clip.clipHalfSize, clip.cornerRadii);
        color.a *= saturate(0.5 - dist);
    }

    if (item.shaderType == FUIShaderType::Texture)
    {
        float4 sampleColor = _Textures[item.textureIndex].Sample(_Samplers[item.samplerIndex], input.uv);
        color *= sampleColor;
    }
    else if (item.shaderType == FUIShaderType::LinearGradient)
    {
        float angle = item.data[0];

        for (int i = 0; i < item.gradientStopCount - 1; i++)
        {
            int idx = item.gradientStartIndex + i;

            const FUIGradientStop left = _GradientStops[idx];
            const FUIGradientStop right = _GradientStops[idx + 1];

            if (left.position <= uv.x && uv.x < right.position)
            {
                float t = clamp01((uv.x - left.position) / (right.position - left.position));
                float4 sampleColor = lerp(UnpackColor(left.packedColor), UnpackColor(right.packedColor), t);
                color *= sampleColor;
                break;
            }
        }
    }

    return color;
}

#endif
