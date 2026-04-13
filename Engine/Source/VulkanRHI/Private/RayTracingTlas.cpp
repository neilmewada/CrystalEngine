#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	RayTracingTlas::RayTracingTlas(VulkanDevice* device, const RHI::RayTracingTlasDescriptor& desc)
		: RHI::RayTracingTlas(desc)
		, device(device)
	{
		return;

		instanceCount = desc.initialInstances.GetSize();

		const VkPhysicalDeviceAccelerationStructurePropertiesKHR& accelProps = device->GetAccelerationStructureProperties();

		VkDeviceAddress tlasInstancesBufferAddress = 0;

		{
			Array<VkAccelerationStructureInstanceKHR> instanceData{};
			instanceData.Reserve(instanceCount);

			for (const RHI::RayTracingTlasInstance& instance : desc.initialInstances)
			{
				VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
				addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
				addrInfo.accelerationStructure = static_cast<Vulkan::RayTracingBlas*>(instance.blas)->GetAccelerationStructure()->GetHandle();
				
				u64 blasDeviceAddress = device->vkGetAccelerationStructureDeviceAddressKHR(device->GetHandle(), &addrInfo);

				VkAccelerationStructureInstanceKHR& instanceInfo = instanceData.EmplaceBack();
				instanceInfo.mask = instance.instanceMask;
				instanceInfo.instanceCustomIndex = instance.instanceID;
				
				memcpy(instanceInfo.transform.matrix[0], instance.transform.rows[0].xyzw, sizeof(float) * 4);
				memcpy(instanceInfo.transform.matrix[1], instance.transform.rows[1].xyzw, sizeof(float) * 4);
				memcpy(instanceInfo.transform.matrix[2], instance.transform.rows[2].xyzw, sizeof(float) * 4);
				
				instanceInfo.accelerationStructureReference = blasDeviceAddress;
				instanceInfo.flags = instance.transparent ? VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR : 0;

				instanceInfo.instanceShaderBindingTableRecordOffset = instance.hitGroupIndex;
			}

			RHI::BufferDescriptor tlasInstancesBufferDesc{};
			tlasInstancesBufferDesc.name = "Tlas Instances Buffer";
			tlasInstancesBufferDesc.bindFlags = BufferBindFlags::RayTracingAccelerationStructure | BufferBindFlags::StructuredBuffer;
			tlasInstancesBufferDesc.bufferSize = sizeof(VkAccelerationStructureInstanceKHR) * instanceCount;
			tlasInstancesBufferDesc.defaultHeapType = MemoryHeapType::Default;
			tlasInstancesBufferDesc.structureByteStride = sizeof(VkAccelerationStructureInstanceKHR);

			tlasInstancesBuffer = new Vulkan::Buffer(device, tlasInstancesBufferDesc);

			tlasInstancesBuffer->UploadData({.dataSize = tlasInstancesBufferDesc.bufferSize, .startOffsetInBuffer = 0,
				.data = instanceData.GetData() });

			VkBufferDeviceAddressInfo addressInfo = {};
			addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			addressInfo.pNext = nullptr;
			addressInfo.buffer = tlasInstancesBuffer->GetBuffer();
			tlasInstancesBufferAddress = device->vkGetBufferDeviceAddressKHR(device->GetHandle(), &addressInfo);
		}

		geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.pNext = nullptr;
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geometry.geometry.instances.pNext = nullptr;
		geometry.geometry.instances.data.deviceAddress = tlasInstancesBufferAddress;
		geometry.geometry.instances.arrayOfPointers = VK_FALSE;

		buildInfo = {};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.pNext = nullptr;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.srcAccelerationStructure = nullptr;

		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
		buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		device->vkGetAccelerationStructureBuildSizesKHR(
			device->GetHandle(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&buildInfo,
			&instanceCount,
			&buildSizesInfo);

		buildSizesInfo.accelerationStructureSize = Memory::AlignUp(buildSizesInfo.accelerationStructureSize, 256);
		buildSizesInfo.buildScratchSize = Memory::AlignUp(buildSizesInfo.buildScratchSize, accelProps.minAccelerationStructureScratchOffsetAlignment);

		RHI::BufferDescriptor scratchBufferDesc{};
		scratchBufferDesc.name = "Tlas Scratch Buffer";
		scratchBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer | RHI::BufferBindFlags::RayTracingScratchBuffer;
		scratchBufferDesc.bufferSize = buildSizesInfo.buildScratchSize;
		scratchBufferDesc.alignment = accelProps.minAccelerationStructureScratchOffsetAlignment;

		scratchBuffer = new Vulkan::Buffer(device, scratchBufferDesc);

		RHI::BufferDescriptor tlasBufferDesc{};
		tlasBufferDesc.name = "TLAS Buffer";
		tlasBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer | RHI::BufferBindFlags::RayTracingAccelerationStructure;
		tlasBufferDesc.bufferSize = buildSizesInfo.accelerationStructureSize;

		tlasBuffer = (Vulkan::Buffer*)RHI::gDynamicRHI->CreateBuffer(tlasBufferDesc);

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.pNext = nullptr;
		accelerationStructureCreateInfo.buffer = tlasBuffer->GetBuffer();
		accelerationStructureCreateInfo.size = buildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.offset = 0;
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		
		accelerationStructure = new RayTracingAccelerationStructure(device, accelerationStructureCreateInfo);

		buildInfo.dstAccelerationStructure = accelerationStructure->GetHandle();

		VkBufferDeviceAddressInfo addressInfo = {};
		addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		addressInfo.pNext = nullptr;
		addressInfo.buffer = scratchBuffer->GetBuffer();
		
		buildInfo.scratchData.deviceAddress = device->vkGetBufferDeviceAddressKHR(device->GetHandle(), &addressInfo);

		offsetInfo = {};
		offsetInfo.primitiveCount = instanceCount;
	}

	RayTracingTlas::~RayTracingTlas()
	{
		delete accelerationStructure; accelerationStructure = nullptr;
		delete tlasInstancesBuffer; tlasInstancesBuffer = nullptr;
		
		delete tlasBuffer; tlasBuffer = nullptr;
		delete scratchBuffer; scratchBuffer = nullptr;
	}

	void RayTracingTlas::SetInstances(u32 numInstances, RayTracingTlasInstance* instances)
	{
		return;

		Array<VkAccelerationStructureInstanceKHR> instanceData{};
		instanceCount = numInstances;

		instanceData.Reserve(instanceCount);

		for (int i = 0; i < numInstances; i++)
		{
			const RayTracingTlasInstance& instance = instances[i];

			VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
			addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			addrInfo.accelerationStructure = static_cast<Vulkan::RayTracingBlas*>(instance.blas)->GetAccelerationStructure()->GetHandle();

			u64 blasDeviceAddress = device->vkGetAccelerationStructureDeviceAddressKHR(device->GetHandle(), &addrInfo);

			VkAccelerationStructureInstanceKHR& instanceInfo = instanceData.EmplaceBack();
			instanceInfo.mask = instance.instanceMask;
			instanceInfo.instanceCustomIndex = instance.instanceID;

			memcpy(instanceInfo.transform.matrix[0], instance.transform.rows[0].xyzw, sizeof(float) * 4);
			memcpy(instanceInfo.transform.matrix[1], instance.transform.rows[1].xyzw, sizeof(float) * 4);
			memcpy(instanceInfo.transform.matrix[2], instance.transform.rows[2].xyzw, sizeof(float) * 4);

			instanceInfo.accelerationStructureReference = blasDeviceAddress;
			instanceInfo.flags = instance.transparent ? VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR : 0;

			instanceInfo.instanceShaderBindingTableRecordOffset = instance.hitGroupIndex;
		}
	}

} // namespace CE::Vulkan
