#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    PipelineLayout::~PipelineLayout()
    {
        if (pipelineLayout)
        {
            vkDestroyPipelineLayout(device->GetHandle(), pipelineLayout, VULKAN_CPU_ALLOCATOR);
            pipelineLayout = nullptr;
        }

        for (VkDescriptorSetLayout setLayout : setLayouts)
        {
            if (setLayout == emptySetLayout)
                continue;

            vkDestroyDescriptorSetLayout(device->GetHandle(), setLayout, VULKAN_CPU_ALLOCATOR);
        }
        setLayouts.Clear();

        if (emptySetLayout)
        {
            vkDestroyDescriptorSetLayout(device->GetHandle(), emptySetLayout, VULKAN_CPU_ALLOCATOR);
            emptySetLayout = nullptr;
        }
    }

    bool PipelineLayout::IsCompatibleWith(PipelineLayout* other)
    {
        for (const auto& [set, bindingList] : setLayoutBindingsMap)
        {
            if (!other->setLayoutBindingsMap.KeyExists(set))
                return false;
            if (bindingList.GetSize() != other->setLayoutBindingsMap[set].GetSize())
                return false;

            const auto& otherBindingList = other->setLayoutBindingsMap[set];

            for (int i = 0; i < bindingList.GetSize(); i++)
            {
                if (bindingList[i].binding != otherBindingList[i].binding)
                    return false;
                if (bindingList[i].descriptorCount != otherBindingList[i].descriptorCount)
                    return false;
                if (bindingList[i].descriptorType != otherBindingList[i].descriptorType)
                    return false;
                if (bindingList[i].stageFlags != otherBindingList[i].stageFlags)
                    return false;
                if (bindingList[i].pImmutableSamplers != nullptr &&
                    otherBindingList[i].pImmutableSamplers == nullptr)
                    return false;
                if (bindingList[i].pImmutableSamplers == nullptr &&
                    otherBindingList[i].pImmutableSamplers != nullptr)
                    return false;
            }
        }

        return true;
    }

    Pipeline::Pipeline(VulkanDevice* device, const PipelineDescriptor& desc) : RHI::RHIResource(RHI::ResourceType::Pipeline), name(desc.name)
    {
        this->device = device;
        
        for (const auto& layout : desc.srgLayouts)
        {
            srgLayouts[layout.srgType] = layout;
        }

        auto srgManager = device->GetShaderResourceManager();
        setLayoutBindingsMap = {};
        setLayoutBindingsByName = {};
        setLayouts = {};
        VkResult result;
        int lowestSetNumber = RHI::Limits::Pipeline::MaxShaderResourceGroupCount;
        int highestSetNumber = -1;

        {
            VkDescriptorSetLayoutCreateInfo emptySetCI{};
            emptySetCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            emptySetCI.bindingCount = 0;
            emptySetCI.pBindings = nullptr;

            vkCreateDescriptorSetLayout(device->GetHandle(), &emptySetCI, VULKAN_CPU_ALLOCATOR, &emptySetLayout);
        }

        for (const RHI::ShaderResourceGroupLayout& srgLayout : desc.srgLayouts)
        {
            int setNumber = srgManager->GetDescriptorSetNumber(srgLayout.srgType);

            if (setNumber < lowestSetNumber)
                lowestSetNumber = setNumber;
            if (setNumber > highestSetNumber)
                highestSetNumber = setNumber;
        }
        
        for (int setNumber = 0; setNumber <= highestSetNumber; setNumber++)
        {
            bool found = false;

            for (const RHI::ShaderResourceGroupLayout& srgLayout : desc.srgLayouts)
            {
                if (srgManager->GetDescriptorSetNumber(srgLayout.srgType) != setNumber)
                    continue;

                found = true;

                List<VkDescriptorBindingFlags> bindingFlags{};
                bindingFlags.Reserve(srgLayout.variables.GetSize());

                for (int i = 0; i < srgLayout.variables.GetSize(); i++)
                {
                    const auto& variable = srgLayout.variables[i];

                    VkDescriptorSetLayoutBinding layoutBinding{};
                    layoutBinding.binding = variable.bindingSlot;
                    layoutBinding.descriptorCount = variable.arrayCount;
                    if (layoutBinding.descriptorCount == 0) // Dynamic size arrays will have descriptor count set as 0
                    {
                        bindingFlags.Add(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
                    }
                    else
                    {
                        bindingFlags.Add(0);
                    }
#if !PLATFORM_DESKTOP
                    CE_ASSERT(layoutBinding.descriptorCount > 0, "Unbounded descriptor arrays are supported only on windows!");
#endif

                    layoutBinding.stageFlags = 0;
                    if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Vertex))
                    {
                        layoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
                    }
                    if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Fragment))
                    {
                        layoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
                    }
                    if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Geometry))
                    {
                        layoutBinding.stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
                    }
                    if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Tessellation))
                    {
                        layoutBinding.stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    }
                    if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Compute))
                    {
                        layoutBinding.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
                    }

                    switch (variable.type)
                    {
                    case RHI::ShaderResourceType::ConstantBuffer:
                        layoutBinding.descriptorType = variable.usesDynamicOffset
                            ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                            : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        break;
                    case RHI::ShaderResourceType::StructuredBuffer:
                    case RHI::ShaderResourceType::RWStructuredBuffer:
                        layoutBinding.descriptorType = variable.usesDynamicOffset
                            ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
                            : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        break;
                    case RHI::ShaderResourceType::Texture1D:
                    case RHI::ShaderResourceType::Texture2D:
                    case RHI::ShaderResourceType::Texture2DArray:
                    case RHI::ShaderResourceType::Texture3D:
                    case RHI::ShaderResourceType::TextureCube:
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        break;
                    case RHI::ShaderResourceType::RWTexture2D:
                    case RHI::ShaderResourceType::RWTexture3D:
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        break;
                    case RHI::ShaderResourceType::SamplerState:
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                        break;
                    case RHI::ShaderResourceType::SubpassInput:
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                        break;
                    default:
                        continue;
                    }

                    setLayoutBindingsMap[setNumber].Add(layoutBinding);

                    setLayoutBindingsByName[variable.name] = layoutBinding;
                }

                // TODO: Implement variable size arrays

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlagsInfo.bindingCount = bindingFlags.GetSize();
                bindingFlagsInfo.pBindingFlags = bindingFlags.GetData();

                VkDescriptorSetLayoutCreateInfo setLayoutCI{};
                setLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                setLayoutCI.bindingCount = setLayoutBindingsMap[setNumber].GetSize();
                setLayoutCI.pBindings = setLayoutBindingsMap[setNumber].GetData();
                setLayoutCI.pNext = &bindingFlagsInfo;

                VkDescriptorSetLayout setLayout = nullptr;
                result = vkCreateDescriptorSetLayout(device->GetHandle(), &setLayoutCI, VULKAN_CPU_ALLOCATOR, &setLayout);
                if (result == VK_SUCCESS)
                {
                    setLayouts.Add(setLayout);
                }

                break;
            }

            // Empty descriptor sets
            if (!found)
            {
                setLayouts.Add(emptySetLayout);
            }
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = setLayouts.GetSize();
        pipelineLayoutCI.pSetLayouts = setLayouts.GetData();
        pipelineLayoutCI.pushConstantRangeCount = 0;
        pipelineLayoutCI.pPushConstantRanges = nullptr;

        if (desc.rootConstantLayout.NotEmpty())
        {
            VkPushConstantRange range{};
            range.offset = 0;
            range.size = 0;

            for (const auto& memberType: desc.rootConstantLayout)
            {
	            switch (memberType)
	            {
	            case ShaderStructMemberType::None: // Should never happen
		            break;
	            case ShaderStructMemberType::UInt:
                    range.size += sizeof(u32);
		            break;
	            case ShaderStructMemberType::Int:
                    range.size += sizeof(s32);
		            break;
	            case ShaderStructMemberType::Float:
                    range.size += sizeof(f32);
		            break;
	            case ShaderStructMemberType::Float2:
                    range.size += sizeof(Vec2);
		            break;
	            case ShaderStructMemberType::Float3:
                    range.size += sizeof(Vec3);
		            break;
	            case ShaderStructMemberType::Float4:
                    range.size += sizeof(Vec4);
		            break;
	            case ShaderStructMemberType::Float4x4:
                    range.size += sizeof(Matrix4x4);
		            break;
	            case ShaderStructMemberType::Struct: // Should never happen
		            break;
	            }
            }

            if (EnumHasFlag(desc.rootConstantShaderStages, ShaderStage::Vertex))
                range.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
            if (EnumHasFlag(desc.rootConstantShaderStages, ShaderStage::Fragment))
                range.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

            if (range.size > 0)
	        {
		        pipelineLayoutCI.pushConstantRangeCount = 1;
            	pipelineLayoutCI.pPushConstantRanges = &range;

                pushConstantRanges.Add(range);
	        }
        }

        result = vkCreatePipelineLayout(device->GetHandle(), &pipelineLayoutCI, VULKAN_CPU_ALLOCATOR, &pipelineLayout);
        if (result != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create Vulkan Pipeline Layout!");
        }
    }

    Pipeline::~Pipeline()
    {
        if (pipeline)
        {
            vkDestroyPipeline(device->GetHandle(), pipeline, VULKAN_CPU_ALLOCATOR);
            pipeline = nullptr;
        }

        if (pipelineCache)
        {
            vkDestroyPipelineCache(device->GetHandle(), pipelineCache, VULKAN_CPU_ALLOCATOR);
            pipelineCache = nullptr;
        }
    }

} // namespace CE::Vulkan
