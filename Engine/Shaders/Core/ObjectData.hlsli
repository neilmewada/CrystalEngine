#ifndef __OBJECT_DATA_HLSL__
#define __OBJECT_DATA_HLSL__

#pragma shader_feature USE_INSTANCING

struct ObjectData
{
    float4x4 modelMatrix;
};

#if USE_INSTANCING

StructuredBuffer<ObjectData> _ObjectData : SRG_PerObject(t0);

#define MODEL_MATRIX(input) _ObjectData[input.instanceId].modelMatrix
#define INSTANCING() uint instanceId : SV_InstanceID;

#else // !USE_INSTANCING

ConstantBuffer<ObjectData> _ObjectData : SRG_PerObject(b0);

#define MODEL_MATRIX(input) _ObjectData.modelMatrix
#define INSTANCING()

#endif // !USE_INSTANCING

#define LOCAL_TO_CLIP_SPACE(localSpace, vertexInput) mul(mul(localSpace, MODEL_MATRIX(vertexInput)), _ViewProjectionMatrix)

#define LOCAL_TO_WORLD_SPACE(localSpace, vertexInput) mul(localSpace, MODEL_MATRIX(vertexInput))

#define WORLD_TO_CLIP_SPACE(worldSpace) mul(worldSpace, _ViewProjectionMatrix)


#endif // __OBJECT_DATA_HLSL__
