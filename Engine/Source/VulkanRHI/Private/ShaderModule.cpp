
#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	ShaderModule::ShaderModule(Device* device, const RHI::ShaderModuleDescriptor& desc)
		: device(device)
	{
		this->stage = desc.stage;
		this->name = desc.debugName;
		this->isValid = false;

		if (desc.byteSize == 0 || desc.byteCode == nullptr)
			return;

		VkShaderModuleCreateInfo shaderCI{};
		shaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCI.pCode = (const uint32_t*)desc.byteCode;
		shaderCI.codeSize = desc.byteSize;

		hash = CalculateHash(desc.byteCode, desc.byteSize);

		VkResult result = vkCreateShaderModule(device->GetHandle(), &shaderCI, VULKAN_CPU_ALLOCATOR, &shaderModule);
		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create Vulkan shader module. Error code {}", (int)result);
			return;
		}

#if PLATFORM_DESKTOP && !CE_BUILD_RELEASE
		device->SetObjectDebugName((uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, name.GetCString());
#endif

		isValid = true;
	}

	ShaderModule::~ShaderModule()
	{
		isValid = false;

		if (shaderModule != nullptr)
		{
			vkDestroyShaderModule(device->GetHandle(), shaderModule, VULKAN_CPU_ALLOCATOR);
			shaderModule = nullptr;
		}
	}

} // namespace CE


