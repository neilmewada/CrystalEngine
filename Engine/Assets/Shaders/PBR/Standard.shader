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
                float3 worldPos : TEXCOORD0;
                float3 normal : TEXCOORD1;
                float2 uv : TEXCOORD2;
                float3 tangent : TEXCOORD3;
                float3 bitangent : TEXCOORD4;
                float4 clipPos : TEXCOORD5;
            };

            #if VERTEX

            PSInput VertMain(VSInput input)
            {
                PSInput output;
                output.position = LOCAL_TO_CLIP_SPACE(float4(input.position, 1.0), input);
                output.worldPos = LOCAL_TO_WORLD_SPACE(float4(input.position, 1.0), input).xyz;
                output.normal = LOCAL_TO_WORLD_SPACE(float4(input.normal, 0), input).xyz;
                output.tangent = LOCAL_TO_WORLD_SPACE(float4(input.tangent.xyz, 0), input).xyz;
                output.bitangent = -cross(output.normal, output.tangent);
                output.uv = input.uv;
                return output;
            }
            #endif

            #if FRAGMENT

            cbuffer _Material : SRG_PerMaterial(b0)
            {
                float4 _Albedo;
                float  _Metallic;
                float  _Roughness;
                float  _NormalStrength;
                float  _AmbientOcclusion;
            };

            Texture2D<float4> _AlbedoTex : SRG_PerMaterial(t1);
            Texture2D<float> _RoughnessTex : SRG_PerMaterial(t2);
            Texture2D<float4> _NormalTex : SRG_PerMaterial(t3);
            Texture2D<float> _MetallicTex : SRG_PerMaterial(t4);

            struct TileFrustum 
            { 
                float4 planes[6]; // plane.xyzw = (n.xyz, d); dot(n,P) + d == 0 is inside
            };

            inline float3 ScreenToWorldSpace(float2 screenPos, float ndcZ, in float4x4 invViewProj)
            {
                float2 ndcXY = screenPos / pixelResolution * 2.0 - 1.0;

                float4 clip = float4(ndcXY, ndcZ, 1);
                float4 worldH = mul(clip, invViewProj);
                return worldH.xyz / worldH.w;
            }

            inline float3 ScreenToViewSpace(float2 screenPos, float ndcZ, in float4x4 invProj)
            {
                float2 ndcXY = screenPos / pixelResolution * 2.0 - 1.0;

                float4 clip = float4(ndcXY, ndcZ, 1);
                float4 viewH = mul(clip, invProj);
                return viewH.xyz / viewH.w;
            }

            inline float3 NDCToViewSpace(float2 ndcXY, float ndcZ, in float4x4 invProj)
            {
                float4 clip = float4(ndcXY, ndcZ, 1);
                float4 viewH = mul(clip, invProj);
                return viewH.xyz / viewH.w;
            }

            float4 MakePlane(float3 a, float3 b, float3 c)
            {
                float3 n = normalize(cross(b - a, c - a));
                float d = -dot(n, a);
                return float4(n, d);
            }

            TileFrustum CreateTileFrustum(uint2 tilePos, in float4x4 invViewProj, in float4x4 invProj)
            {
                const uint width = (uint)pixelResolution.x;
                const uint height = (uint)pixelResolution.y;
                const uint tilesX = (width + tileSizeX - 1) / tileSizeX;
                const uint tilesY = (height + tileSizeY - 1) / tileSizeY;
                //const uint tileId = ty * tilesX + tx;

                TileFrustum fr;
                
                float2 pBase = float2(tilePos * uint2(tileSizeX, tileSizeY));

                // NDC coordinates range: [-1, 1]
                float2 p00 = pBase / pixelResolution.xy * 2.0f - 1.0f;
                float2 p10 = float2(pBase.x + tileSizeX, pBase.y) / pixelResolution.xy * 2.0f - 1.0f;
                float2 p01 = float2(pBase.x, pBase.y + tileSizeY) / pixelResolution.xy * 2.0f - 1.0f;
                float2 p11 = float2(pBase.x + tileSizeX, pBase.y + tileSizeY) / pixelResolution.xy * 2.0f - 1.0f;

                float3 topLeftFront = NDCToViewSpace(p00, 0.0, invProj);
                float3 topLeftBack = NDCToViewSpace(p00, 1.0, invProj);
                float3 topRightFront = NDCToViewSpace(p10, 0.0, invProj);
                float3 topRightBack = NDCToViewSpace(p10, 1.0, invProj);
                float3 bottomLeftFront = NDCToViewSpace(p01, 0.0, invProj);
                float3 bottomLeftBack = NDCToViewSpace(p01, 1.0, invProj);
                float3 bottomRightFront = NDCToViewSpace(p11, 0.0, invProj);
                float3 bottomRightBack = NDCToViewSpace(p11, 1.0, invProj);

                // fr.planes = left, top, right, bottom, front, back
                fr.planes[0] = MakePlane(topLeftFront, topLeftBack, bottomLeftFront);
                fr.planes[1] = MakePlane(topLeftFront, topRightFront, topRightBack);
                fr.planes[2] = MakePlane(topRightFront, bottomRightFront, topRightBack);
                fr.planes[3] = MakePlane(bottomLeftFront, bottomLeftBack, bottomRightFront);

                fr.planes[4] = MakePlane(topRightFront, topLeftFront, bottomLeftFront); // front
                fr.planes[5] = MakePlane(topRightBack, bottomRightBack, topLeftBack); // back

                return fr;
            }

            float4 FragMain(PSInput input) : SV_TARGET
            {
                float3 diffuse = 0;
                float3 specular = 0;
                float3 vertNormal = normalize(input.normal);
                float3 viewDir = normalize(viewPosition - input.worldPos);
                float3 tangent = normalize(input.tangent);
                float3 bitangent = normalize(input.bitangent);

                float4 normalMapSample = _NormalTex.Sample(_DefaultSampler, input.uv);
                float3 tangentSpaceNormal = normalize(normalMapSample.xyz * 2.0 - 1.0);

                float3x3 tangentToWorld = float3x3(tangent, bitangent, vertNormal);
                float3 normal = normalize(mul(tangentSpaceNormal, tangentToWorld));

                MaterialInput material;
                material.albedo = GammaToLinear(_Albedo.rgb * _AlbedoTex.Sample(_DefaultSampler, input.uv).rgb);
                material.metallic = _Metallic * _MetallicTex.Sample(_DefaultSampler, input.uv);
                material.roughness = _Roughness * _RoughnessTex.Sample(_DefaultSampler, input.uv);
                material.ambient = _AmbientOcclusion;

                float3 Lo = float3(0, 0, 0);
                
                uint i = 0;
                for (i = 0; i < totalDirectionalLights; i++)
                {
                    LightInput light;
                    light.lightRadiance = _DirectionalLights[i].colorAndIntensity.rgb * _DirectionalLights[i].colorAndIntensity.a;
                    light.lightDir = -_DirectionalLights[i].direction;
                    light.normal = normal;
                    light.viewDir = viewDir;
                    light.halfway = normalize(viewDir + light.lightDir);

                    float shadow = 0.0;
                    if (_DirectionalLights[i].shadow > 0)
                    {
                        float4 lightSpacePos = mul(float4(input.worldPos, 1.0), _DirectionalLights[i].lightSpaceMatrix);
                        shadow = CalculateDirectionalShadow(lightSpacePos, dot(vertNormal, light.lightDir));
                    }
                    shadow = clamp(shadow, 0, 1);

                    Lo += CalculateBRDF(light, material) * (1.0 - shadow);
                }

                const float2 screenPos = input.position.xy;

                // Derive tiling from current render target size
                const uint width = (uint)pixelResolution.x;
                const uint height = (uint)pixelResolution.y;
                const uint tilesX = (width + tileSizeX - 1) / tileSizeX;
                const uint tilesY = (height + tileSizeY - 1) / tileSizeY;

                // Map global thread to tile coordinates
                const uint tx = (uint)(screenPos.x / tileSizeX);
                const uint ty = (uint)(screenPos.y / tileSizeY);
                const uint tileId = ty * tilesX + tx;

                const uint localLightBase = _TileHeaders[tileId].x;
                const uint numLocalLights = _TileHeaders[tileId].y;

                for (i = 0; i < numLocalLights; i++)
                {
                    uint lightIndex = _LightIndexPool[localLightBase + i];
                    LocalLightType lightType = _LocalLights[lightIndex].type;

                    LightInput light;
                    float3 luminosity = _LocalLights[lightIndex].colorAndIntensity.rgb * _LocalLights[lightIndex].colorAndIntensity.a;

                    if (lightType == LocalLightType_Point)
                    {
                        light.lightDir = _LocalLights[lightIndex].worldPosAndRange.xyz - input.worldPos;
                        float distance = length(light.lightDir);
                        float attenuation = AttenuateCusp(distance, _LocalLights[lightIndex].worldPosAndRange.w, 2.0, 4.0);
                        light.lightDir = normalize(light.lightDir);
                        light.lightRadiance = luminosity * attenuation;
                    }
                    else
                    {
                        continue;
                    }
                    
                    light.normal = normal;
                    light.viewDir = viewDir;
                    light.halfway = normalize(viewDir + light.lightDir);

                    float shadow = 0.0;

                    Lo += CalculateBRDF(light, material) * (1.0 - shadow);
                }

                float3 color = ComputeSkyboxIBL(material, normal, viewDir);

                color += Lo;
                color = color / (color + float3(1.0, 1.0, 1.0) * 0.5); // HDR Tonemapping (optional)

                color = LinearToGamma(color); // Convert to Gamma space

                return float4(color, 1.0);
            }

            #endif

            ENDHLSL
        }
    }
}