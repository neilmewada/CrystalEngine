
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
    const FUIDrawItemFlags flags = item.flags;

    // - Clipping -

    if (item.clipRectIndex >= 0)
    {
        FUIClipRect clip = _ClipRects[item.clipRectIndex];
        float2 clipLocalPos = mul(float4(input.layerPos, 0.0, 1.0), clip.clipInverseTransform).xy;
        float dist = SDFRoundedRect(clipLocalPos, clip.clipHalfSize, clip.cornerRadii);
        color.a *= saturate(0.5 - dist);
    }

    // - Shading -

    if (item.shaderType == FUIShaderType::Texture)
    {
        float2 sampleUVMin = float2(item.data[0], item.data[1]);
        float2 sampleUVMax = float2(item.data[2], item.data[3]);
        float2 fitOffset = float2(item.data[4], item.data[5]);
        float2 fitSize = float2(item.data[6], item.data[7]);
        float2 halfTexel = float2(item.data[8], item.data[9]);

        const bool contain = flags & ImageFitContain;
        const bool cover = flags & ImageFitCover;
        const bool tileX = flags & TextureTileX;
        const bool tileY = flags & TextureTileY;
        const bool noTile = !tileX && !tileY;

        float2 imgUV = (input.uv - fitOffset) / fitSize;

        sampleUVMin += halfTexel;
        sampleUVMax -= halfTexel;

        float2 sampleUV = lerp(sampleUVMin, sampleUVMax, imgUV);
        float4 sampleColor;

        if ((!tileX && (imgUV.x < 0 || imgUV.x > 1)) || (!tileY && (imgUV.y < 0 || imgUV.y > 1)))
            sampleColor = float4(0, 0, 0, 0);
        else
            sampleColor = _Textures[item.textureIndex].Sample(_Samplers[item.samplerIndex], sampleUV);

        color *= sampleColor;
    }
    else if (item.shaderType == FUIShaderType::LinearGradient)
    {
        float angle = item.data[0];
        uv = fmod(uv, 1.0f);

        float2 dir = float2(cos(angle), sin(angle));
        float range = abs(dir.x) + abs(dir.y);
        float gradientT = dot(uv - 0.5, dir) / range + 0.5;

        for (int i = 0; i < item.gradientStopCount - 1; i++)
        {
            int idx = item.gradientStartIndex + i;

            const FUIGradientStop left = _GradientStops[idx];
            const FUIGradientStop right = _GradientStops[idx + 1];

            if (left.position <= gradientT && gradientT < right.position)
            {
                float segT = clamp01((gradientT - left.position) / (right.position - left.position));
                float4 sampleColor = lerp(UnpackColor(left.packedColor), UnpackColor(right.packedColor), segT);
                color *= sampleColor;
                break;
            }
        }
    }

    return color;
}

#endif
