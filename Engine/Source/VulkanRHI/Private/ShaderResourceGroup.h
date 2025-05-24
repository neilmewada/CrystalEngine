#pragma once

namespace CE::Vulkan
{
	class VulkanDescriptorSet;
	class ShaderResourceGroup;
	class MergedShaderResourceGroup;

	class ShaderResourceManager
	{
	public:

		ShaderResourceManager(VulkanDevice* device);
		~ShaderResourceManager();

		inline u32 GetMaxBoundSets() const { return maxBoundDescriptorSets; }
        
        inline bool IsSharedDescriptorSet(int set) { return setNumberToSrgs[set].GetSize() > 1; }
		bool IsSharedDescriptorSet(RHI::SRGType srgType);

		int GetDescriptorSetNumber(RHI::SRGType srgType);

		bool ShouldCreateDescriptorSetForSRG(RHI::SRGType srgType);

		VkDescriptorType GetDescriptorType(ShaderResourceType shaderResourceType, bool usesDynamicOffset = false);

		MergedShaderResourceGroup* FindOrCreateMergedSRG(u32 srgCount, ShaderResourceGroup** srgs);

		MergedShaderResourceGroup* CreateMergedSRG(u32 srgCount, ShaderResourceGroup** srgs);
		void RemoveMergedSRG(MergedShaderResourceGroup* srg);
		void OnSRGDestroyed(ShaderResourceGroup* srg);

		void DestroyQueuedSRG();

		void QueueDestroy(DescriptorSet* descriptorSet);

	private:
        
        struct SRGSlot
        {
            RHI::SRGType srgType{};
            int set;
        };
        
		VulkanDevice* device = nullptr;
        Array<SRGSlot> srgSlots{};
        
        HashMap<RHI::SRGType, SRGSlot> srgTypeToDescriptorSet{};
		HashMap<int, Array<SRGSlot>> setNumberToSrgs{};
		
		u32 maxBoundDescriptorSets = 0;

		SharedMutex queuedDestroySetMutex{};
		List<DescriptorSet*> queuedDestroySets{};

		HashMap<SIZE_T, MergedShaderResourceGroup*> mergedSRGsByHash{};

		/// @brief HashMap of Merged SRG by each source SRG. Used to manage lifetime of Merged SRG.
		/// If any one of the source SRG that comprises the Merged SRG is destroyed, the Merged SRG should be destroyed.
		HashMap<Vulkan::ShaderResourceGroup*, Array<MergedShaderResourceGroup*>> mergedSRGsBySourceSRG{};

		Array<Vulkan::ShaderResourceGroup*> destroyQueue{};

		friend class DescriptorSet;
		friend class ShaderResourceGroup;
		friend class MergedShaderResourceGroup;
	};

	class ShaderResourceGroup : public RHI::ShaderResourceGroup
	{
	public:

		ShaderResourceGroup(VulkanDevice* device, const RHI::ShaderResourceGroupLayout& srgLayout);

		virtual ~ShaderResourceGroup();

		virtual bool IsMerged() const { return false; }

		virtual bool HasVariable(const Name& variableName) override;

		virtual bool Bind(Name name, RHI::BufferView bufferView) override;
		virtual bool Bind(Name name, RHI::Texture* texture) override;
		virtual bool Bind(Name name, RHI::TextureView* textureView) override;
		virtual bool Bind(Name name, RHI::Sampler* sampler) override;

		virtual bool Bind(Name name, u32 count, RHI::BufferView* bufferViews) override;
		virtual bool Bind(Name name, u32 count, RHI::Texture** textures) override;
		virtual bool Bind(Name name, u32 count, RHI::TextureView** textureViews) override;
		virtual bool Bind(Name name, u32 count, RHI::Sampler** samplers) override;

		virtual bool Bind(u32 imageIndex, Name name, RHI::BufferView bufferView) override;
		virtual bool Bind(u32 imageIndex, Name name, RHI::Texture* texture) override;
		virtual bool Bind(u32 imageIndex, Name name, RHI::TextureView* textureView) override;
		virtual bool Bind(u32 imageIndex, Name name, RHI::Sampler* sampler) override;

		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::BufferView* bufferViews) override;
		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::Texture** textures) override;
		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::TextureView** textureViews) override;
		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::Sampler** samplers) override;
		
		inline int GetSetNumber() const { return setNumber; }

		inline DescriptorSet* GetDescriptorSet() const { return descriptorSets[currentImageIndex]; }
		inline VkDescriptorSetLayout GetDescriptorSetLayout() const { return setLayout; }

		void Compile() override;

		inline bool IsFailed() const
		{
			return failed;
		}

		void FlushBindings() override;

		virtual SIZE_T GetHash() const { return (SIZE_T)this; }

		inline void SetCommitted(bool set)
		{
			isCommitted = set;
		}

	protected:

		ShaderResourceGroup(VulkanDevice* device);

		// Queue the native DescriptorSet* to be destroyed later
		void QueueDestroy();

		void UpdateBindings();

		bool failed = false;
		bool needsRecompile = true;

		VulkanDevice* device = nullptr;
		DescriptorPool* pool = nullptr;
		VkDescriptorPool allocPool = nullptr;

		u32 currentImageIndex = 0;
		int setNumber = -1;
		//DescriptorSet* descriptorSet = nullptr;
		StaticArray<DescriptorSet*, RHI::Limits::MaxSwapChainImageCount> descriptorSets{};
		VkDescriptorSetLayout setLayout = nullptr;

		Array<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		HashMap<Name, int> bindingSlotsByVariableName{};
        HashMap<Name, SRGVariableDescriptor> variableDescriptorsByName{};
		
		HashMap<Name, VkDescriptorSetLayoutBinding> variableBindingsByName{};
		HashMap<int, VkDescriptorSetLayoutBinding> variableBindingsBySlot{};

		StaticArray<HashMap<int, List<VkDescriptorBufferInfo>>, RHI::Limits::MaxSwapChainImageCount> bufferInfosBoundBySlot{};
		StaticArray<HashMap<int, List<VkDescriptorImageInfo>>, RHI::Limits::MaxSwapChainImageCount> imageInfosBoundBySlot{};

		friend class GraphicsPipelineState;
		friend class CommandList;
        friend class VulkanDescriptorSet;
		friend class MergedShaderResourceGroup;
		friend class ShaderResourceManager;

	};

} // namespace CE
