#ifndef __LIGHTING_DATA_HLSL__
#define __LIGHTING_DATA_HLSL__

#define MAX_DIRECTIONAL_LIGHTS 8

struct DirectionalLight
{
    float4x4 lightSpaceMatrix;
    float4 direction;
    float4 colorAndIntensity;
    float temperature;
    uint shadow;
};

enum LocalLightType : uint
{
    LocalLightType_Point,
    LocalLightType_Spot
};

struct LocalLightData
{
    // xyz = world position, w = range (meters)
    float4 worldPosAndRange;

    // xyz = color * intensity (linear RGB), w = (optional: unused or intensity if you prefer)
    float4 colorAndIntensity;

    // xyz = world-space spot direction (normalized), w = unused
    float4 spotLightDirection;

    // radians (outer cone half-angle for spots). For points, set <= 0
    float  spotLightAngle;

    // 0 = point, 1 = spot
    LocalLightType type;
};

#endif