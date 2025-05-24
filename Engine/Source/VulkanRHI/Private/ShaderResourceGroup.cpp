#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	ShaderResourceManager::ShaderResourceManager(VulkanDevice* device)
		: device(device)
	{
		const auto& limits = device->GetDeviceLimits();
		maxBoundDescriptorSets =  limits.maxBoundDescriptorSets;
        
        if (maxBoundDescriptorSets == 4)
        {
            srgSlots.Add({ RHI::SRGType::PerScene, 0 });
            srgSlots.Add({ RHI::SRGType::PerView, 0 });
            srgSlots.Add({ RHI::SRGType::PerPass, 0 });
            srgSlots.Add({ RHI::SRGType::PerSubPass, 0 });
            srgSlots.Add({ RHI::SRGType::PerMaterial, 1 });
            srgSlots.Add({ RHI::SRGType::PerObject, 2 });
            srgSlots.Add({ RHI::SRGType::PerDraw, 3 });
        }
        else if (maxBoundDescriptorSets == 5)
        {
            srgSlots.Add({ RHI::SRGType::PerScene, 0 });
            srgSlots.Add({ RHI::SRGType::PerView, 0 });
            srgSlots.Add({ RHI::SRGType::PerPass, 1 });
            srgSlots.Add({ RHI::SRGType::PerSubPass, 1 });
            srgSlots.Add({ RHI::SRGType::PerMaterial, 2 });
            srgSlots.Add({ RHI::SRGType::PerObject, 3 });
            srgSlots.Add({ RHI::SRGType::PerDraw, 4 });
        }
        else if (maxBoundDescriptorSets == 6)
        {
            srgSlots.Add({ RHI::SRGType::PerScene, 0 });
            srgSlots.Add({ RHI::SRGType::PerView, 1 });
            srgSlots.Add({ RHI::SRGType::PerPass, 2 });
            srgSlots.Add({ RHI::SRGType::PerSubPass, 2 });
            srgSlots.Add({ RHI::SRGType::PerMaterial, 3 });
            srgSlots.Add({ RHI::SRGType::PerObject, 4 });
            srgSlots.Add({ RHI::SRGType::PerDraw, 5 });
        }
        else if (maxBoundDescriptorSets >= 7)
        {
            srgSlots.Add({ RHI::SRGType::PerScene, 0 });
            srgSlots.Add({ RHI::SRGType::PerView, 1 });
            srgSlots.Add({ RHI::SRGType::PerPass, 2 });
            srgSlots.Add({ RHI::SRGType::PerSubPass, 3 });
            srgSlots.Add({ RHI::SRGType::PerMaterial, 4 });
            srgSlots.Add({ RHI::SRGType::PerObject, 5 });
            srgSlots.Add({ RHI::SRGType::PerDraw, 6 });
        }
        
        for (const SRGSlot& slot : srgSlots)
        {
			srgTypeToDescriptorSet[slot.srgType] = slot;
			setNumberToSrgs[slot.set].Add(slot);
        }
	}

	ShaderResourceManager::~ShaderResourceManager()
	{
		vkDeviceWaitIdle(device->GetHandle());

		for (auto [hash, srg] : mergedSRGsByHash)
		{
			delete srg;
		}
		mergedSRGsByHash.Clear();

		LockGuard<SharedMutex> lock{ queuedDestroySetMutex };

		for (auto set : queuedDestroySets)
		{
			delete set;
		}
		queuedDestroySets.Clear();
	}

	bool ShaderResourceManager::IsSharedDescriptorSet(RHI::SRGType srgType)
	{
		if (!srgTypeToDescriptorSet.KeyExists(srgType))
			return false;

		int set = srgTypeToDescriptorSet[srgType].set;
		return IsSharedDescriptorSet(set);
	}

	int ShaderResourceManager::GetDescriptorSetNumber(RHI::SRGType srgType)
	{
		if (!srgTypeToDescriptorSet.KeyExists(srgType))
			return false;

		return srgTypeToDescriptorSet[srgType].set;
	}

	bool ShaderResourceManager::ShouldCreateDescriptorSetForSRG(RHI::SRGType srgType)
	{
		if (!srgTypeToDescriptorSet.KeyExists(srgType))
			return false;

		int set = srgTypeToDescriptorSet[srgType].set;
		const auto& array = setNumberToSrgs[set];
		if (array.IsEmpty())
			return false;

		return array.GetLast().srgType == srgType;
	}

	VkDescriptorType ShaderResourceManager::GetDescriptorType(ShaderResourceType shaderResourceType, bool usesDynamicOffset)
	{
		switch (shaderResourceType)
		{
		case ShaderResourceType::ConstantBuffer:
			return usesDynamicOffset ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case ShaderResourceType::StructuredBuffer:
		case ShaderResourceType::RWStructuredBuffer:
			return usesDynamicOffset ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case ShaderResourceType::Texture1D:
		case ShaderResourceType::Texture2D:
		case ShaderResourceType::Texture2DArray:
		case ShaderResourceType::Texture3D:
		case ShaderResourceType::TextureCube:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case ShaderResourceType::RWTexture2D:
		case ShaderResourceType::RWTexture3D:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case ShaderResourceType::SubpassInput:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case ShaderResourceType::SamplerState:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		}

		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	MergedShaderResourceGroup* ShaderResourceManager::FindOrCreateMergedSRG(u32 srgCount, ShaderResourceGroup** srgs)
	{
		SIZE_T mergedSRGHash = 0;
		FixedArray<Vulkan::ShaderResourceGroup*, RHI::Limits::Pipeline::MaxShaderResourceGroupCount> srgArray{};
		
		for (int i = 0; i < srgCount; i++)
		{
			srgArray.Add(srgs[i]);
		}
        
		std::sort(srgArray.begin(), srgArray.end(),
            [](Vulkan::ShaderResourceGroup* a, Vulkan::ShaderResourceGroup* b) -> bool
			{
				return (int)a->GetSRGType() <= (int)b->GetSRGType();
			});

		mergedSRGHash = (SIZE_T)srgArray[0];
		for (int i = 1; i < srgArray.GetSize(); i++)
		{
			mergedSRGHash = GetCombinedHash(mergedSRGHash, (SIZE_T)srgArray[i]);
		}

		if (mergedSRGsByHash.KeyExists(mergedSRGHash) && mergedSRGsByHash[mergedSRGHash] != nullptr)
		{
			return mergedSRGsByHash[mergedSRGHash];
		}

		return CreateMergedSRG(srgArray.GetSize(), srgArray.GetData());
	}

	MergedShaderResourceGroup* ShaderResourceManager::CreateMergedSRG(u32 srgCount, ShaderResourceGroup** srgs)
	{
		if (srgCount)
			return nullptr;

		MergedShaderResourceGroup* mergedSRG = new MergedShaderResourceGroup(device, srgCount, srgs);
		if (mergedSRG->GetMergedHash() == 0)
		{
			delete mergedSRG;
			return nullptr;
		}
		
		mergedSRGsByHash[mergedSRG->GetMergedHash()] = mergedSRG;

		return mergedSRG;
	}

	void ShaderResourceManager::RemoveMergedSRG(MergedShaderResourceGroup* mergedSRG)
	{
		if (!mergedSRG)
			return;

		mergedSRGsByHash.Remove(mergedSRG->GetMergedHash());

		for (auto sourceSRG : mergedSRG->combinedSRGs)
		{
			mergedSRGsBySourceSRG[sourceSRG].Remove(mergedSRG);
		}
	}

	void ShaderResourceManager::OnSRGDestroyed(ShaderResourceGroup* srg)
	{
		if (!mergedSRGsBySourceSRG.KeyExists(srg))
			return;

		Array<MergedShaderResourceGroup*> srgsToDestroy{};
		srgsToDestroy.Resize(mergedSRGsBySourceSRG[srg].GetSize());

		int i = 0;
		
		// Destroy all Merged SRGs created from the given source 'srg'
		for (MergedShaderResourceGroup* mergedSRG : mergedSRGsBySourceSRG[srg])
		{
			// Get all source SRGs used to create 'mergeSRG'
			for (ShaderResourceGroup* sourceSRG : mergedSRG->combinedSRGs)
			{
				// Remove reference to 'mergeSRG' from all the source SRGs used to create it.
				mergedSRGsBySourceSRG[sourceSRG].Remove(mergedSRG);
			}

			mergedSRGsByHash.Remove(mergedSRG->GetMergedHash());
			srgsToDestroy[i++] = mergedSRG;
		}

		mergedSRGsBySourceSRG[srg].Clear();
		mergedSRGsBySourceSRG.Remove(srg);
		
		for (auto destroySrg : srgsToDestroy)
		{
			destroySrg->combinedSRGs.Clear();
			delete destroySrg;
		}
	}

	void ShaderResourceManager::DestroyQueuedSRG()
	{
		ZoneScoped;

		LockGuard<SharedMutex> lock{ queuedDestroySetMutex };

		for (int i = queuedDestroySets.GetSize() - 1; i >= 0; i--)
		{
			if (queuedDestroySets[i]->usageCount <= 0)
			{
				delete queuedDestroySets[i];
				queuedDestroySets.RemoveAt(i);
			}
		}
	}

	void ShaderResourceManager::QueueDestroy(DescriptorSet* descriptorSet)
	{
		if (descriptorSet)
		{
			LockGuard<SharedMutex> lock{ queuedDestroySetMutex };

			queuedDestroySets.Add(descriptorSet);
		}
	}

    ShaderResourceGroup::ShaderResourceGroup(VulkanDevice* device, const RHI::ShaderResourceGroupLayout& srgLayout)
		: device(device)
    {
		this->srgType = srgLayout.srgType;
		this->srgLayout = srgLayout;
		auto srgManager = device->GetShaderResourceManager();
		pool = device->GetDescriptorPool();
		setNumber = srgManager->GetDescriptorSetNumber(srgType);

        variableDescriptorsByName.Clear();
		setLayoutBindings.Clear();
		bindingSlotsByVariableName.Clear();
		variableBindingsByName.Clear();
		variableBindingsBySlot.Clear();

		List<VkDescriptorBindingFlags> bindingFlags{};
		bindingFlags.Reserve(srgLayout.variables.GetSize());

		for (const RHI::SRGVariableDescriptor& variable : srgLayout.variables)
		{
			setLayoutBindings.Add({});
            variableDescriptorsByName[variable.name] = variable;
            
			VkDescriptorSetLayoutBinding& entry = setLayoutBindings.GetLast();
			entry.binding = variable.bindingSlot;
			entry.descriptorCount = variable.arrayCount;
			entry.descriptorType = srgManager->GetDescriptorType(variable.type, variable.usesDynamicOffset);
			entry.stageFlags = 0;

			if (entry.descriptorCount == 0) // Dynamic size arrays will have descriptor count set as 0
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

			if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Vertex))
			{
				entry.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
			}
			if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Fragment))
			{
				entry.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Compute))
			{
				entry.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
			}
			if (EnumHasFlag(variable.shaderStages, RHI::ShaderStage::Tessellation))
			{
				entry.stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			}

			bindingSlotsByVariableName[variable.name] = variable.bindingSlot;
			variableBindingsByName[variable.name] = entry;
			variableBindingsBySlot[variable.bindingSlot] = entry;
		}

		// TODO: Implement variable size arrays

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindingFlags.GetSize();
		bindingFlagsInfo.pBindingFlags = bindingFlags.GetData();

		VkDescriptorSetLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCI.bindingCount = setLayoutBindings.GetSize();
		layoutCI.pBindings = setLayoutBindings.GetData();
		layoutCI.pNext = &bindingFlagsInfo;

		auto result = vkCreateDescriptorSetLayout(device->GetHandle(), &layoutCI, VULKAN_CPU_ALLOCATOR, &setLayout);
		if (result != VK_SUCCESS)
		{
			failed = true;
			CE_LOG(Error, All, "Failed to create vulkan descriptor set layout");
			return;
		}
    }

    ShaderResourceGroup::~ShaderResourceGroup()
	{
		//vkDeviceWaitIdle(device->GetHandle());
		vkDestroyDescriptorSetLayout(device->GetHandle(), setLayout, VULKAN_CPU_ALLOCATOR);
		setLayout = nullptr;

		if (ShaderResourceManager* srgManager = device->GetShaderResourceManager())
		{
			srgManager->OnSRGDestroyed(this);
		}
		
		QueueDestroy();
	}

	void ShaderResourceGroup::FlushBindings()
	{
		if (!isCompiled)
		{
			Compile();
		}
		else if (needsRecompile) // Free current DescriptorSet and create new one
		{
			QueueDestroy();
			Compile();
		}
	}

	ShaderResourceGroup::ShaderResourceGroup(VulkanDevice* device)
		: device(device)
	{
		pool = device->GetDescriptorPool();
	}

	bool ShaderResourceGroup::HasVariable(const Name& variableName)
	{
		return bindingSlotsByVariableName.KeyExists(variableName);
	}

	bool ShaderResourceGroup::Bind(Name name, RHI::BufferView bufferView)
	{
		if (!bufferView.GetBuffer())
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, bufferView);

			needsRecompile = true;
		}
		
		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::BufferView* bufferViews)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, count, bufferViews);
		}
		
		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, RHI::Texture* rhiTexture)
	{
		if (!rhiTexture)
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;
        
		int bindingSlot = bindingSlotsByVariableName[name];
		Vulkan::Texture* texture = (Vulkan::Texture*)rhiTexture;

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, rhiTexture);

			needsRecompile = true;
		}

		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, RHI::TextureView* rhiTextureView)
	{
		if (!rhiTextureView)
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, rhiTextureView);

			needsRecompile = true;
		}

		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::Texture** textures)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, count, textures);
		}
		
		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::TextureView** textureViews)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, count, textureViews);
		}

		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, RHI::Sampler* rhiSampler)
	{
		if (!rhiSampler)
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];
		Vulkan::Sampler* sampler = (Vulkan::Sampler*)rhiSampler;

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, rhiSampler);

			needsRecompile = true;
		}
		
		return true;
	}

	bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::Sampler** samplers)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];

		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			Bind(i, name, count, samplers);
		}

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::BufferView bufferView)
	{
		if (!bufferView.GetBuffer())
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
		{
			CE_LOG(Warn, All, "Variable named {} does not exist in SRG", name);
			return false;
		}

		int bindingSlot = bindingSlotsByVariableName[name];
		int i = imageIndex;

		auto size = bufferView.GetByteCount();

		VkDescriptorBufferInfo bufferWrite{};
		bufferWrite.buffer = (VkBuffer)bufferView.GetBuffer()->GetHandle();
		bufferWrite.offset = bufferView.GetByteOffset();
		bufferWrite.range = size > 0 ? size : bufferView.GetBuffer()->GetBufferSize();

		if (bufferInfosBoundBySlot[i][bindingSlot].GetSize() == 1 &&
			bufferInfosBoundBySlot[i][bindingSlot][0].buffer == bufferWrite.buffer &&
			bufferInfosBoundBySlot[i][bindingSlot][0].offset == bufferWrite.offset &&
			bufferInfosBoundBySlot[i][bindingSlot][0].range == bufferWrite.range)
		{
			return true;
		}

		bufferInfosBoundBySlot[i][bindingSlot].Clear();
		bufferInfosBoundBySlot[i][bindingSlot].Add(bufferWrite);

		needsRecompile = true;

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::Texture* rhiTexture)
	{
		if (!rhiTexture)
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
		{
			CE_LOG(Warn, All, "Variable named {} does not exist in SRG", name);
			return false;
		}

		int i = imageIndex;
		int bindingSlot = bindingSlotsByVariableName[name];
		Vulkan::Texture* texture = (Vulkan::Texture*)rhiTexture;

		if (!variableBindingsBySlot.KeyExists(bindingSlot))
			return false;

		// Do not make changes if the same image is already bound!

		if (imageInfosBoundBySlot[i][bindingSlot].GetSize() == 1 &&
			imageInfosBoundBySlot[i][bindingSlot][0].imageView == texture->GetImageView())
		{
			return true;
		}

		const VkDescriptorSetLayoutBinding& binding = variableBindingsBySlot[bindingSlot];

		VkImageLayout expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		switch (binding.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			expectedLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		default:
			break;
		}

		VkDescriptorImageInfo imageWrite{};
		imageWrite.imageLayout = expectedLayout;
		imageWrite.imageView = texture->GetImageView();
		imageWrite.sampler = nullptr;

		imageInfosBoundBySlot[i][bindingSlot].Clear();
		imageInfosBoundBySlot[i][bindingSlot].Add(imageWrite);

		needsRecompile = true;

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::TextureView* rhiTextureView)
	{
		if (!rhiTextureView)
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int i = imageIndex;
		int bindingSlot = bindingSlotsByVariableName[name];
		Vulkan::TextureView* textureView = (Vulkan::TextureView*)rhiTextureView;

		if (!variableBindingsBySlot.KeyExists(bindingSlot))
			return false;

		// Do not make changes if the same image is already bound!

		if (imageInfosBoundBySlot[i][bindingSlot].GetSize() == 1 &&
			imageInfosBoundBySlot[i][bindingSlot][0].imageView == textureView->GetImageView())
		{
			return true;
		}

		const VkDescriptorSetLayoutBinding& binding = variableBindingsBySlot[bindingSlot];

		VkImageLayout expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		switch (binding.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			expectedLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		default:
			break;
		}

		VkDescriptorImageInfo imageWrite{};
		imageWrite.imageLayout = expectedLayout;
		imageWrite.imageView = textureView->GetImageView();
		imageWrite.sampler = nullptr;

		imageInfosBoundBySlot[i][bindingSlot].Clear();
		imageInfosBoundBySlot[i][bindingSlot].Add(imageWrite);

		needsRecompile = true;

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::Sampler* rhiSampler)
	{
		if (!rhiSampler)
			return false;
		if (!bindingSlotsByVariableName.KeyExists(name))
		{
			CE_LOG(Warn, All, "Variable named {} does not exist in SRG", name);
			return false;
		}

		int i = imageIndex;
		int bindingSlot = bindingSlotsByVariableName[name];
		Vulkan::Sampler* sampler = (Vulkan::Sampler*)rhiSampler;

		if (imageInfosBoundBySlot[i][bindingSlot].GetSize() == 1 &&
			imageInfosBoundBySlot[i][bindingSlot][0].sampler == sampler->GetVkHandle())
		{
			return true;
		}

		VkDescriptorImageInfo imageWrite{};
		imageWrite.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageWrite.imageView = VK_NULL_HANDLE;
		imageWrite.sampler = sampler->GetVkHandle();

		imageInfosBoundBySlot[i][bindingSlot].Clear();
		imageInfosBoundBySlot[i][bindingSlot].Add(imageWrite);

		needsRecompile = true;

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::BufferView* bufferViews)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
		{
			CE_LOG(Warn, All, "Variable named {} does not exist in SRG", name);
			return false;
		}

		int bindingSlot = bindingSlotsByVariableName[name];
		int i = imageIndex;

		bufferInfosBoundBySlot[i][bindingSlot].Clear();

		for (int j = 0; j < count; j++)
		{
			auto size = bufferViews[j].GetByteCount();

			VkDescriptorBufferInfo bufferWrite{};
			bufferWrite.buffer = (VkBuffer)bufferViews[j].GetBuffer()->GetHandle();
			bufferWrite.offset = bufferViews[j].GetByteOffset();
			bufferWrite.range = size > 0 ? size : bufferViews[j].GetBuffer()->GetBufferSize();

			bufferInfosBoundBySlot[i][bindingSlot].Add(bufferWrite);
		}

		needsRecompile = true;

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::Texture** textures)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
		{
			CE_LOG(Warn, All, "Variable named {} does not exist in SRG", name);
			return false;
		}

		int bindingSlot = bindingSlotsByVariableName[name];
		int i = imageIndex;

		if (!variableBindingsBySlot.KeyExists(bindingSlot))
			return false;

		const VkDescriptorSetLayoutBinding& binding = variableBindingsBySlot[bindingSlot];

		VkImageLayout expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		switch (binding.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			expectedLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		default:
			break;
		}

		imageInfosBoundBySlot[i][bindingSlot].Clear();
		imageInfosBoundBySlot[i][bindingSlot].Reserve(count);

		for (int j = 0; j < count; j++)
		{
			Vulkan::Texture* texture = (Vulkan::Texture*)textures[j];

			VkDescriptorImageInfo imageWrite{};
			imageWrite.imageLayout = expectedLayout;
			imageWrite.imageView = texture->GetImageView();
			imageWrite.sampler = nullptr;

			imageInfosBoundBySlot[i][bindingSlot].Add(imageWrite);
		}

		needsRecompile = true;

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::TextureView** textureViews)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];
		int i = imageIndex;

		if (!variableBindingsBySlot.KeyExists(bindingSlot))
			return false;

		const VkDescriptorSetLayoutBinding& binding = variableBindingsBySlot[bindingSlot];

		VkImageLayout expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		switch (binding.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			expectedLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		default:
			break;
		}

		if (count != imageInfosBoundBySlot[i][bindingSlot].GetSize())
		{
			needsRecompile = true;
		}
		else
		{
			for (int j = 0; j < count; ++j)
			{
				if (j >= imageInfosBoundBySlot[i][bindingSlot].GetSize())
				{
					needsRecompile = true;
					break;
				}

				Vulkan::TextureView* texture = (Vulkan::TextureView*)textureViews[j];

				if (imageInfosBoundBySlot[i][bindingSlot][j].imageView != texture->GetImageView())
				{
					needsRecompile = true;
					break;
				}
			}
		}

		// Do not make changes if the array elements and their order are same as before!

		if (!needsRecompile)
			return true;

		imageInfosBoundBySlot[i][bindingSlot].Clear();

		for (int j = 0; j < count; j++)
		{
			Vulkan::TextureView* texture = (Vulkan::TextureView*)textureViews[j];

			VkDescriptorImageInfo imageWrite{};
			imageWrite.imageLayout = expectedLayout;
			imageWrite.imageView = texture->GetImageView();
			imageWrite.sampler = nullptr;

			imageInfosBoundBySlot[i][bindingSlot].Add(imageWrite);
		}

		return true;
	}

	bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::Sampler** samplers)
	{
		if (!bindingSlotsByVariableName.KeyExists(name))
			return false;

		int bindingSlot = bindingSlotsByVariableName[name];
		int i = imageIndex;

		imageInfosBoundBySlot[i][bindingSlot].Clear();

		for (int j = 0; j < count; j++)
		{
			Vulkan::Sampler* sampler = (Vulkan::Sampler*)samplers[j];
			if (sampler == nullptr)
				continue;

			VkDescriptorImageInfo imageWrite{};
			imageWrite.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageWrite.imageView = VK_NULL_HANDLE;
			imageWrite.sampler = sampler->GetVkHandle();

			imageInfosBoundBySlot[i][bindingSlot].Add(imageWrite);
		}

		needsRecompile = true;

		return true;
	}

	void ShaderResourceGroup::Compile()
	{
		if (isCompiled)
			return;

		for (int i = 0; i < descriptorSets.GetSize(); i++)
		{
			Name dynamicArrayName = "";
			u32 dynamicArraySize = 0;

			for (const auto& variable : srgLayout.variables)
			{
				if (variable.arrayCount == 0)
				{
					switch (variable.type)
					{
					case ShaderResourceType::None:
						continue;
					case ShaderResourceType::ConstantBuffer:
					case ShaderResourceType::StructuredBuffer:
					case ShaderResourceType::RWStructuredBuffer:
						dynamicArraySize = bufferInfosBoundBySlot[i][variable.bindingSlot].GetSize();
						break;
					case ShaderResourceType::Texture1D:
					case ShaderResourceType::Texture2D:
					case ShaderResourceType::Texture2DArray:
					case ShaderResourceType::Texture3D:
					case ShaderResourceType::TextureCube:
					case ShaderResourceType::RWTexture2D:
					case ShaderResourceType::RWTexture3D:
					case ShaderResourceType::RWTexture2DArray:
					case ShaderResourceType::SamplerState:
					case ShaderResourceType::SubpassInput:
						dynamicArraySize = imageInfosBoundBySlot[i][variable.bindingSlot].GetSize();
						break;
					}

					dynamicArrayName = variable.name;
					break;
				}
			}

			if (dynamicArrayName.IsValid())
				descriptorSets[i] = new DescriptorSet(device, setLayout, srgLayout, dynamicArraySize);
			else
				descriptorSets[i] = new DescriptorSet(device, setLayout, srgLayout);
		}

		UpdateBindings();

		isCompiled = true;
		needsRecompile = false;
	}

	void ShaderResourceGroup::QueueDestroy()
	{
		auto srgManager = device->GetShaderResourceManager();
		if (!srgManager)
			return;

		for (int i = 0; i < descriptorSets.GetSize(); i++)
		{
			srgManager->QueueDestroy(descriptorSets[i]);
			descriptorSets[i] = nullptr;
		}

		isCommitted = isCompiled = false;
	}

	void ShaderResourceGroup::UpdateBindings()
	{
		for (int i = 0; i < descriptorSets.GetSize(); i++)
		{
			DescriptorSet* descriptorSet = descriptorSets[i];
			if (descriptorSet == nullptr)
				continue;

			Array<VkWriteDescriptorSet> writes{};
			writes.Resize(bufferInfosBoundBySlot[i].GetSize() + imageInfosBoundBySlot[i].GetSize());
			int idx = 0;

			for (const auto& [slot, bufferWrites] : bufferInfosBoundBySlot[i])
			{
				if (!variableBindingsBySlot.KeyExists(slot))
					continue;

				VkDescriptorSetLayoutBinding& variable = variableBindingsBySlot[slot];

				VkWriteDescriptorSet& write = writes[idx++];
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.descriptorType = variable.descriptorType;
				write.dstArrayElement = 0;
				write.descriptorCount = variable.descriptorCount; // Array count
				if (write.descriptorCount == 0) // Dynamic sized array
				{
					write.descriptorCount = bufferWrites.GetSize();
				}
				write.dstBinding = variable.binding;
				write.dstSet = descriptorSet->GetHandle();
				write.pBufferInfo = bufferWrites.GetData();
			}

			for (const auto& [slot, imageWrites] : imageInfosBoundBySlot[i])
			{
				if (!variableBindingsBySlot.KeyExists(slot))
					continue;

				VkDescriptorSetLayoutBinding& variable = variableBindingsBySlot[slot];

				if (variable.descriptorCount == 0 && imageWrites.IsEmpty())
					continue;

				VkWriteDescriptorSet& write = writes[idx++];
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.descriptorType = variable.descriptorType;
				write.dstArrayElement = 0;
				write.descriptorCount = imageWrites.GetSize(); // Array count
				if (write.descriptorCount == 0) // Dynamic sized array
				{
					//write.descriptorCount = imageWrites.GetSize();
				}
				write.dstBinding = variable.binding;
				write.dstSet = descriptorSet->GetHandle();
				write.pImageInfo = imageWrites.GetData();
			}
			
			vkUpdateDescriptorSets(device->GetHandle(), idx, writes.GetData(),
				0, nullptr);
		}
	}

} // namespace CE

