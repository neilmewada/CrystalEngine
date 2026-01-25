Shader "Utils/Blit"
{
    Properties
    {
        _InputTexture ("Input Texture", Tex2D) = "white"
    }

    SubShader
    {
        Tags {
            "Blend" = "One,Zero"
        }

        Pass
        {
            Name "Blit"
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

            Texture2D<float4> _InputTexture : SRG_PerMaterial(t0);
            SamplerState _InputSampler : SRG_PerMaterial(s1);

            float4 FragMain(PSInput input) : SV_Target
            {
                float4 color = _InputTexture.Sample(_InputSampler, input.uv);
                return color;
            }

            #endif

            ENDHLSL
        }
    }
}