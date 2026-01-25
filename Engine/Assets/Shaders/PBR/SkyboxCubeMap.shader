Shader "PBR/Standard"
{
    Properties
    {
        _CubeMap("CubeMap", TexCube) = "white"
    }

    SubShader
    {
        Tags {
            "Blend" = "SrcAlpha,OneMinusSrcAlpha"
        }

        Pass
        {
            Name "Opaque"
            Tags {
                "Vertex"="VertMain", "Fragment"="FragMain", "DrawListTag"="skybox"
            }
            ZWrite Off
            ZTest Off
            Cull Off

            HLSLPROGRAM
            
            #include "OpaqueIncludes.hlsli"

            struct VSInput
            {
                float3 position : POSITION;
            };

            struct PSInput
            {
                float4 position : SV_POSITION;
                float3 worldPos : TEXCOORD0;
            };

            #if VERTEX

            PSInput VertMain(VSInput input)
            {
                PSInput o;
                o.worldPos = LOCAL_TO_WORLD_SPACE(float4(input.position, 1.0), input).xyz;
                o.position = WORLD_TO_CLIP_SPACE(float4(o.worldPos, 1.0));
                return o;
            }
            #endif

            #if FRAGMENT

            TextureCube<float4> _CubeMap : SRG_PerMaterial(t0);
            SamplerState _CubeMapSampler : SRG_PerMaterial(s1);

            float4 FragMain(PSInput input) : SV_TARGET
            {
                float3 direction = normalize(input.worldPos - viewPosition);
                float3 color = _CubeMap.SampleLevel(_CubeMapSampler, direction, 0.0).rgb;
                color = color / (color + float3(1.0, 1.0, 1.0) * 0.5); // HDR tonemapping (optional)
                return float4(color, 1);
            }

            #endif

            ENDHLSL
        }
    }
}