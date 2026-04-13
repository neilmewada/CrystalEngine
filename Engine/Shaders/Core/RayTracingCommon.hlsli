#pragma once

struct RayPayload
{
    float3 color;
    float  hitT;         // RayTCurrent() on hit, -1 on miss
    uint   isShadowRay;  // use uint, bool has alignment issues in payloads
    uint   shadowed;
    uint   simplified;   // skip full PBR for secondary rays
};
