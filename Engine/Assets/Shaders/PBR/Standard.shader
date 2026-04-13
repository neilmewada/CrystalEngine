Shader "PBR/Standard"
{
    Properties
    {
        _Albedo ("Albedo", Color) = (1, 1, 1, 1)
        _Metallic ("Metallic", Float) = 1.0
        _Roughness ("Roughness", Float) = 1.0
        _NormalStrength ("Normal Strength", Float) = 1.0
        _AmbientOcclusion ("Ambient Occlusion", Float) = 1.0

        _AlbedoTex ("Albedo Map", Tex2D) = "white"
        _RoughnessTex ("Roughness Map", Tex2D) = "white"
        _NormalTex ("Normal Map", Tex2D) = "bump"
        _MetallicTex ("Metallic Map", Tex2D) = "white"
    }

    SubShader
    {

        Pass
        {
            Name "Depth"
            Tags {
                "Vertex"="VertMain", "DrawListTag"="depth", "Blend" = "SrcAlpha,OneMinusSrcAlpha"
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
                "Vertex"="VertMain", "DrawListTag"="shadow", "Blend" = "SrcAlpha,OneMinusSrcAlpha"
            }
            ZWrite On
            ZTest LEqual
            Cull Off

            HLSLPROGRAM

            #include "Depth.hlsli"

            ENDHLSL
        }

        Pass
        {
            Name "Opaque"
            Tags { 
                "Vertex"="VertMain", "Fragment"="FragMain", "DrawListTag"="opaque", "Blend" = "SrcAlpha,OneMinusSrcAlpha"
            }
            ZWrite Off
            ZTest LEqual
            Cull Back

            HLSLPROGRAM
            
            #include "Opaque.hlsli"

            ENDHLSL
        }

        HitGroup
        {
            Name "RayHit"
            Tags
            {
                "ClosestHit" = "ClosestHitMain"
            }

            HLSLPROGRAM

            #include "Core/RayTracingCommon.hlsli"
            
            [shader("closesthit")]
            void ClosestHitMain(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attr)
            {
                payload.color = float4(1.0, 1.0, 1.0, 1.0);
            }

            ENDHLSL
        }
    }
}