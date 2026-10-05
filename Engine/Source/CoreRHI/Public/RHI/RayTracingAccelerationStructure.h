#pragma once

namespace CE::RHI
{
    class RayTracingAccelerationStructure;

    enum class RayTracingAccelerationStructureFlags
    {
		None = 0,
		AllowUpdate = BIT(0),
		AllowCompaction = BIT(1),
		FastTrace = BIT(2),
		FastBuild = BIT(3)
    };
	ENUM_CLASS_FLAGS(RayTracingAccelerationStructureFlags);

    enum class RayTracingAccelerationStructureType
    {
	    TopLevel = 0,
        BottomLevel
    };
    ENUM_CLASS(RayTracingAccelerationStructureType);

    struct RayTracingGeometryDescriptor
    {
		VertexAttributeDataType vertexDataType = VertexAttributeDataType::Undefined;
        VertexBufferView vertexBuffer;
        IndexBufferView indexBuffer;
        u32 vertexOffset = 0;
	};

    struct RayTracingBlasDescriptor
    {
        Array<RayTracingGeometryDescriptor> geometries;
		Aabb aabb{}; // aabb will be used if no geometries are provided
		RayTracingAccelerationStructureFlags flags = RayTracingAccelerationStructureFlags::FastTrace;
    };

    struct RayTracingTlasInstance
    {
		u32 instanceID = 0;
		u32 hitGroupIndex = 0;
		u32 instanceMask = 0xFF;
		Matrix4x4 transform = Matrix4x4::Identity();
		bool transparent = false;
        RayTracingAccelerationStructure* blas = nullptr;
	};

    struct RayTracingTlasDescriptor
    {
        u32 maxInstanceCount = 0;
        RayTracingAccelerationStructureFlags flags = RayTracingAccelerationStructureFlags::AllowUpdate | RayTracingAccelerationStructureFlags::FastTrace;
    };
    

    class CORERHI_API RayTracingAccelerationStructure : public RHI::RHIResource, public IDeviceObject
    {
        CE_NO_COPY(RayTracingAccelerationStructure)
    protected:
        RayTracingAccelerationStructure()
            : RHIResource(ResourceType::RayTracingAccelerationStructure)
            , IDeviceObject(DeviceObjectType::RayTracingAccelerationStructure)
        {}

    public:

        RayTracingAccelerationStructureType GetType() const { return accelerationStructureType; }

        RayTracingAccelerationStructureFlags GetFlags() const { return flags; }

    protected:

        RayTracingAccelerationStructureType accelerationStructureType = RayTracingAccelerationStructureType::TopLevel;
        RayTracingAccelerationStructureFlags flags = RayTracingAccelerationStructureFlags::None;

    };
    
} // namespace CE::RHI
