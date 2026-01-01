#pragma once

namespace CE::RHI
{
    enum class RayTracingBuildFlags
    {
		None = 0,
		AllowUpdate = BIT(0),
		AllowCompaction = BIT(1),
		FastTrace = BIT(2),
		FastBuild = BIT(3)
    };
	ENUM_CLASS_FLAGS(RayTracingBuildFlags);

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
		RayTracingBuildFlags buildFlags = RayTracingBuildFlags::FastTrace;
    };

    class CORERHI_API RayTracingBlas : RHI::RHIResource, public IDeviceObject
    {
    protected:
		RayTracingBlas(const RHI::RayTracingBlasDescriptor& desc) 
    	: RHIResource(ResourceType::RayTracingBlas)
			, IDeviceObject(DeviceObjectType::Blas)
    		, geometries(desc.geometries)
		{}

    public:

        const Array<RayTracingGeometryDescriptor>& GetGeometries() const { return geometries; }

    protected:

        Array<RayTracingGeometryDescriptor> geometries;
    };
    
} // namespace CE::RHI
