
#include "VulkanRHIPrivate.h"


namespace CE::Vulkan
{
    static VkSamplerAddressMode ToVkSamplerAddressMode(RHI::SamplerAddressMode mode)
    {
        switch (mode)
        {
		case RHI::SamplerAddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case RHI::SamplerAddressMode::MirroredRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case RHI::SamplerAddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case RHI::SamplerAddressMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
    
	VkFilter Sampler::ToVkFilter(RHI::FilterMode filterMode)
	{
		switch (filterMode)
		{
		case RHI::FilterMode::Linear:
			return VK_FILTER_LINEAR;
		case RHI::FilterMode::Nearest:
			return VK_FILTER_NEAREST;
		case RHI::FilterMode::Cubic:
			if (device->IsDeviceExtensionSupported(VK_EXT_FILTER_CUBIC_EXTENSION_NAME))
				return VK_FILTER_CUBIC_EXT;
			break;
		}
		return VK_FILTER_LINEAR;
	}

    Sampler::Sampler(Device* device, const RHI::SamplerDescriptor& samplerDesc)
        : device(device), borderColor(samplerDesc.borderColor)
    {
        VkSamplerCreateInfo samplerCI{};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.addressModeU = ToVkSamplerAddressMode(samplerDesc.addressModeU);
        samplerCI.addressModeV = ToVkSamplerAddressMode(samplerDesc.addressModeV);
        samplerCI.addressModeW = ToVkSamplerAddressMode(samplerDesc.addressModeW);
        samplerCI.borderColor = (VkBorderColor)samplerDesc.borderColor;
        samplerCI.anisotropyEnable = samplerDesc.enableAnisotropy ? VK_TRUE : VK_FALSE;
        samplerCI.maxAnisotropy = samplerDesc.maxAnisotropy;
        samplerCI.minFilter = ToVkFilter(samplerDesc.samplerFilterMode);
        samplerCI.magFilter = ToVkFilter(samplerDesc.samplerFilterMode);
        samplerCI.minLod = 0;
        samplerCI.maxLod = VK_LOD_CLAMP_NONE;
        samplerCI.compareEnable = VK_FALSE;
        
        if (samplerDesc.samplerFilterMode == RHI::FilterMode::Nearest)
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        else
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		
        if (vkCreateSampler(device->GetHandle(), &samplerCI, VULKAN_CPU_ALLOCATOR, &sampler) != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create vulkan sampler");
        }
    }

    Sampler::~Sampler()
    {
        if (sampler != nullptr)
        {
            vkDestroySampler(device->GetHandle(), sampler, VULKAN_CPU_ALLOCATOR);
        }
    }

} // namespace CE

