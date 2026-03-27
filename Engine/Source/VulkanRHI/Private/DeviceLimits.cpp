#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	DeviceLimits::DeviceLimits(Device* device)
	{
		EnumType* formatEnum = GetStaticEnum<RHI::Format>();

		const VkPhysicalDeviceProperties& gpuProps = device->gpuProperties;
		maxConstantBufferRange = gpuProps.limits.maxUniformBufferRange;
		maxStructuredBufferRange = gpuProps.limits.maxStorageBufferRange;

		minStructuredBufferOffsetAlignment = gpuProps.limits.minStorageBufferOffsetAlignment;

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device->gpu, &features);

		sparseBinding = features.sparseBinding;
		sparseResidency2D = features.sparseResidencyImage2D;
		sparseResidency2DSample2 = features.sparseResidency2Samples;
		sparseResidency2DSample4 = features.sparseResidency4Samples;
		sparseResidency2DSample8 = features.sparseResidency8Samples;
		sparseResidency2DSample16 = features.sparseResidency16Samples;
		sparseResidency3D = features.sparseResidencyImage3D;

		for (int i = 0; formatEnum != nullptr && i < formatEnum->GetConstantsCount(); i++)
		{
			RHI::Format rhiFormat = (RHI::Format)formatEnum->GetConstant(i)->GetValue();
			VkFormat vkFormat = RHIFormatToVkFormat(rhiFormat);
			if (vkFormat == VK_FORMAT_UNDEFINED)
				continue;
			if (vkFormat == VK_FORMAT_D32_SFLOAT)
			{
				String::IsAlphabet('a');
			}

			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(device->GetPhysicalHandle(), vkFormat, &properties);

			imageFormatSupport[rhiFormat].bindFlags = RHI::TextureBindFlags::None;

			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
			{
				imageFormatSupport[rhiFormat].bindFlags |= RHI::TextureBindFlags::ShaderRead;
				imageFormatSupport[rhiFormat].bindFlags |= RHI::TextureBindFlags::SubpassInput;
			}
			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
			{
				imageFormatSupport[rhiFormat].filterMask |= RHI::FilterModeMask::Linear;
			}
			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG)
			{
				imageFormatSupport[rhiFormat].filterMask |= RHI::FilterModeMask::Cubic;
			}

			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
			{
				imageFormatSupport[rhiFormat].bindFlags |= RHI::TextureBindFlags::ShaderWrite;
			}

			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				if (IsDepthStencilFormat(rhiFormat))
					imageFormatSupport[rhiFormat].bindFlags |= RHI::TextureBindFlags::DepthStencil;
				else
					imageFormatSupport[rhiFormat].bindFlags |= RHI::TextureBindFlags::Depth;
			}

			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
			{
				imageFormatSupport[rhiFormat].bindFlags |= RHI::TextureBindFlags::Color;
			}
		}
	}

} // namespace CE::Vulkan
