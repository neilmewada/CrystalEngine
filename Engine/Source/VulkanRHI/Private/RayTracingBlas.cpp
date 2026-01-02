#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	static VkFormat ToVkFormat(RHI::VertexAttributeDataType dataType)
	{
		switch (dataType)
		{
		case VertexAttributeDataType::Float:
			return VK_FORMAT_R32_SFLOAT;
			break;
		case VertexAttributeDataType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
			break;
		case VertexAttributeDataType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case VertexAttributeDataType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case VertexAttributeDataType::Int:
			return VK_FORMAT_R32_SINT;
			break;
		case VertexAttributeDataType::Int2:
			return VK_FORMAT_R32G32_SINT;
			break;
		case VertexAttributeDataType::Int3:
			return VK_FORMAT_R32G32B32_SINT;
			break;
		case VertexAttributeDataType::Int4:
			return VK_FORMAT_R32G32B32A32_SINT;
			break;
		case VertexAttributeDataType::UInt:
			return VK_FORMAT_R32_UINT;
			break;
		case VertexAttributeDataType::UInt2:
			return VK_FORMAT_R32G32_UINT;
			break;
		case VertexAttributeDataType::UInt3:
			return VK_FORMAT_R32G32B32_UINT;
			break;
		case VertexAttributeDataType::UInt4:
			return VK_FORMAT_R32G32B32A32_UINT;
			break;
		case VertexAttributeDataType::Undefined:
			break;
		case VertexAttributeDataType::Char4:
			return VK_FORMAT_R8G8B8A8_SNORM;
			break;
		case VertexAttributeDataType::UChar4:
			return VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}

		return VK_FORMAT_UNDEFINED;
	}

	static VkBuildAccelerationStructureFlagsKHR GetVkAccelerationStructureBuildFlags(RHI::RayTracingBuildFlags buildFlags)
	{
		VkBuildAccelerationStructureFlagsKHR flags = 0;

		if (EnumHasFlag(buildFlags, RayTracingBuildFlags::AllowCompaction))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
		}

		if (EnumHasFlag(buildFlags, RayTracingBuildFlags::AllowUpdate))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		}

		if (EnumHasFlag(buildFlags, RayTracingBuildFlags::FastBuild))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
		}

		if (EnumHasFlag(buildFlags, RayTracingBuildFlags::FastTrace))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		}

		return flags;
	}

	RayTracingBlas::RayTracingBlas(VulkanDevice* device, const RHI::RayTracingBlasDescriptor& desc)
		: RHI::RayTracingBlas(desc), device(device)
	{
		Array<uint32_t> primitiveCounts;

		VkPhysicalDevice physicalDevice = device->GetPhysicalHandle();

		const VkPhysicalDeviceAccelerationStructurePropertiesKHR& accelProps = device->GetAccelerationStructureProperties();

		if (desc.geometries.NotEmpty())
		{
			const auto& geometries = desc.geometries;

			geometryDescriptors.Reserve(geometries.GetSize());
			rangeInfos.Reserve(geometries.GetSize());
			primitiveCounts.Reserve(geometries.GetSize());

			for (const RHI::RayTracingGeometryDescriptor& geometry : geometries)
			{
				VkAccelerationStructureGeometryKHR geometryDesc = {};
				geometryDesc.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				geometryDesc.pNext = nullptr;
				geometryDesc.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				geometryDesc.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				geometryDesc.geometry.triangles.pNext = nullptr;

				VkBuffer vertexBuffer = (VkBuffer)geometry.vertexBuffer.GetBuffer()->GetHandle();
				VkBuffer indexBuffer = (VkBuffer)geometry.indexBuffer.GetBuffer()->GetHandle();

				VkBufferDeviceAddressInfo addressInfo{};
				addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				addressInfo.pNext = nullptr;

				addressInfo.buffer = vertexBuffer;
				VkDeviceAddress vertexAddress = device->vkGetBufferDeviceAddressKHR(device->GetHandle(), &addressInfo) +
					geometry.vertexBuffer.GetByteOffset();

				geometryDesc.geometry.triangles.vertexData.deviceAddress = vertexAddress;
				geometryDesc.geometry.triangles.vertexStride = geometry.vertexBuffer.GetVertexStride();
				geometryDesc.geometry.triangles.maxVertex = static_cast<uint32_t>(geometry.vertexBuffer.GetByteCount() / geometryDesc.geometry.triangles.vertexStride);
				geometryDesc.geometry.triangles.vertexFormat = ToVkFormat(geometry.vertexDataType);

				addressInfo.buffer = indexBuffer;
				VkDeviceAddress indexAddress = device->vkGetBufferDeviceAddressKHR(device->GetHandle(), &addressInfo) +
					geometry.indexBuffer.GetByteOffset();

				geometryDesc.geometry.triangles.indexData.deviceAddress = indexAddress;
				geometryDesc.geometry.triangles.indexType = geometry.indexBuffer.GetIndexFormat() == RHI::IndexFormat::Uint16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
				geometryDesc.geometry.triangles.transformData = {};

				geometryDesc.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
				geometryDescriptors.Add(geometryDesc);

				u32 indexCount = static_cast<u32>(geometry.indexBuffer.GetByteCount() / (geometry.indexBuffer.GetIndexFormat() == RHI::IndexFormat::Uint16 ? sizeof(u16) : sizeof(u32)));
				
				VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
				rangeInfo.firstVertex = geometry.vertexOffset;
				rangeInfo.primitiveOffset = 0;
				rangeInfo.transformOffset = 0;
				rangeInfo.primitiveCount = indexCount / 3;
				rangeInfos.Add(rangeInfo);

				primitiveCounts.Add(rangeInfo.primitiveCount);
			}
		}
		else
		{
			// TODO: AABB for procedurally defined geometries
			return;
		}

		buildInfo = {};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.flags = GetVkAccelerationStructureBuildFlags(desc.buildFlags);
		buildInfo.geometryCount = static_cast<uint32_t>(geometryDescriptors.GetSize());
		buildInfo.pGeometries = geometryDescriptors.GetData();
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.srcAccelerationStructure = nullptr;

		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {};
		buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		
		device->vkGetAccelerationStructureBuildSizesKHR(
			device->GetHandle(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&buildInfo,
			primitiveCounts.GetData(),
			&buildSizesInfo);


		buildSizesInfo.accelerationStructureSize = Memory::AlignUp(buildSizesInfo.accelerationStructureSize, 256);
		buildSizesInfo.buildScratchSize = Memory::AlignUp(buildSizesInfo.buildScratchSize, accelProps.minAccelerationStructureScratchOffsetAlignment);

		RHI::BufferDescriptor scratchBufferDesc{};
		scratchBufferDesc.name = "Scratch Buffer";
		scratchBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer | RHI::BufferBindFlags::RayTracingScratchBuffer;
		scratchBufferDesc.bufferSize = buildSizesInfo.buildScratchSize;
		scratchBufferDesc.alignment = accelProps.minAccelerationStructureScratchOffsetAlignment;

		scratchBuffer = (Vulkan::Buffer*)RHI::gDynamicRHI->CreateBuffer(scratchBufferDesc);

		RHI::BufferDescriptor blasBufferDesc{};
		blasBufferDesc.name = "BLAS Buffer";
		blasBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer | RHI::BufferBindFlags::RayTracingAccelerationStructure;
		blasBufferDesc.bufferSize = buildSizesInfo.accelerationStructureSize;

		blasBuffer = (Vulkan::Buffer*)RHI::gDynamicRHI->CreateBuffer(blasBufferDesc);

		VkAccelerationStructureCreateInfoKHR accelCreateInfo{};
		accelCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelCreateInfo.size = buildSizesInfo.accelerationStructureSize;
		accelCreateInfo.offset = 0;

		VkBufferDeviceAddressInfo addressInfo{};
		addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		addressInfo.buffer = (VkBuffer)blasBuffer->GetHandle();

		accelCreateInfo.buffer = (VkBuffer)blasBuffer->GetHandle();

		accelerationStructure = new RayTracingAccelerationStructure(device, accelCreateInfo);

		buildInfo.dstAccelerationStructure = (VkAccelerationStructureKHR)accelerationStructure->GetHandle();

		addressInfo.buffer = scratchBuffer->GetBuffer();
		buildInfo.scratchData.deviceAddress = device->vkGetBufferDeviceAddressKHR(device->GetHandle(), &addressInfo);
	}

	RayTracingBlas::~RayTracingBlas()
	{
		delete accelerationStructure; accelerationStructure = nullptr;

		delete blasBuffer; blasBuffer = nullptr;
		delete scratchBuffer; scratchBuffer = nullptr;
	}

}
