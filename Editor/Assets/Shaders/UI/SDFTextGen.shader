Shader "UI/SDF Text Generator"
{
    Properties
    {
        
    }

    SubShader
    {
        Tags {
            "Blend" = "SrcAlpha,OneMinusSrcAlpha"
        }

        Pass
        {
            Name "Text"
            Tags { 
                "Vertex"="VertMain", "Fragment"="FragMain"
            }
            ZWrite Off
            ZTest Off
            Cull Off

            HLSLPROGRAM
            
            #include "Core/Macros.hlsli"
            #include "Core/Gamma.hlsli"

            struct VSInput
            {
                float3 position : POSITION;
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
                o.position = float4(input.position, 1.0);
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
                // Sample multiple pixels to get pixel state to smooth out corners
                // Set spread to 0 to disable multiple samples for performance improvement
                const int spread = 1;
                float count = spread * 2 + 1;

                float value = 0.0;

                for (float x = -spread; x <= spread; x++)
                {
                    for (float y = -spread; y <= spread; y++)
                    {
                        float2 localUV = uv + float2(float(x) / w, float(y) / h);
                        value += _FontAtlas.Sample(_FontAtlasSampler, localUV).r;
                        value /= count;
                    }
                }

                return value > 0.25 ? 1 : 0;
            }

            float4 FragMain(PSInput input) : SV_TARGET
            {
                uint w; uint h;
                _FontAtlas.GetDimensions(w, h);

                const int spread = _Spread; // = 7;
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

            ENDHLSL
        }
    }
}