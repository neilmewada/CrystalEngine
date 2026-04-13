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

    struct RayTracingTlasInstance
    {
		u32 instanceID = 0;
		u32 hitGroupIndex = 0;
		u32 instanceMask = 0xFF;
		Matrix4x4 transform = Matrix4x4::Identity();
		bool transparent = false;
		RayTracingBlas* blas = nullptr;
	};

    struct RayTracingTlasDescriptor
    {
		Array<RayTracingTlasInstance> initialInstances;

    };

    class CORERHI_API RayTracingTlas : RHI::RHIResource, public IDeviceObject
    {
	protected:
		RayTracingTlas(const RHI::RayTracingTlasDescriptor& desc)
			: RHIResource(ResourceType::RayTracingTlas)
			, IDeviceObject(DeviceObjectType::Tlas)
		{}

    public:

        virtual void SetInstances(u32 numInstances, RayTracingTlasInstance* instances) = 0;

    protected:


    };
    
} // namespace CE::RHI
