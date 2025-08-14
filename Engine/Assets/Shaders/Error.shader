Shader "Error"
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
            Name "Depth"
            Tags {
                "Vertex"="VertMain", "DrawListTag"="depth"
            }
            ZWrite On
            ZTest LEqual

            HLSLPROGRAM

            #include "Depth.hlsli"

            ENDHLSL
        }

        Pass
        {
            Name "Shadow"
            Tags {
                "Vertex"="VertMain", "DrawListTag"="shadow"
            }
            ZWrite On
            ZTest LEqual

            HLSLPROGRAM

            #include "Depth.hlsli"

            ENDHLSL
        }

        Pass
        {
            Name "Opaque"
            Tags { 
                "Vertex"="VertMain", "Fragment"="FragMain", "DrawListTag"="opaque"
            }
            ZWrite Off
            ZTest LEqual
            Cull Back

            HLSLPROGRAM

            #include "OpaqueIncludes.hlsli"

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
            };

            #if VERTEX

            PSInput VertMain(VSInput input)
            {
                PSInput output;
                output.position = LOCAL_TO_CLIP_SPACE(float4(input.position, 1.0), input);
                return output;
            }
            #endif

            #if FRAGMENT

            float4 FragMain(PSInput input) : SV_TARGET
            {
                return float4(1, 0, 1, 1);
            }

            #endif

            ENDHLSL
        }
    }
}