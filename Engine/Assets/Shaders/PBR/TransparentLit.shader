Shader "PBR/Transparent Lit"
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
        Tags {
            "Blend" = "SrcAlpha,OneMinusSrcAlpha"
        }

        Pass
        {
            Name "Depth"
            Tags {
                "Vertex"="VertMain", "DrawListTag"="depth"
            }
            ZWrite Off

            HLSLPROGRAM

            #include "Depth.hlsli"

            ENDHLSL
        }

        Pass
        {
            Name "Transparent"
            Tags { 
                "Vertex"="VertMain", "Fragment"="FragMain", "DrawListTag"="transparent"
            }
            ZWrite Off
            ZTest LEqual
            Cull Back

            HLSLPROGRAM
            
            #include "Transparent.hlsli"

            ENDHLSL
        }
    }
}