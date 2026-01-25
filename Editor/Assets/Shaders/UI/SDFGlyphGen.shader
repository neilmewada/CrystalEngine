Shader "Fusion/SDF Glyph Generator"
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

            Texture2D<float> _FontGlyph : SRG_PerMaterial(t0);
            SamplerState _FontGlyphSampler : SRG_PerMaterial(s1);

            float4 FragMain(PSInput input) : SV_TARGET
            {
                uint w; uint h;
                _FontGlyph.GetDimensions(w, h);

                const int spread = 7;
                const float stepSize = 0.1;
                
                return float4(0, 0, 0, 1);
            }

            #endif

            ENDHLSL
        }
    }
}