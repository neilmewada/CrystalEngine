#pragma once

namespace CE::Vulkan
{

    class DescriptorPool
    {
    public:
        
        DescriptorPool();
        
        ~DescriptorPool();

        void Init(VulkanDevice* device, u32 initialPoolSize = 128, u32 poolSizeIncrement = 128);
        
		List<VkDescriptorSet> Allocate(u32 numDescriptorSets, const List<VkDescriptorSetLayout>& setLayouts, VkDescriptorPool& outPool);

        List<VkDescriptorSet> Allocate(u32 numDescriptorSets, const List<VkDescriptorSetLayout>& setLayouts, VkDescriptorPool& outPool, 
            const List<uint32_t>& descriptorCounts);

		void Free(const List<VkDescriptorSet>& sets);
        
        void Increment(u32 size);
        
    private:

		SharedMutex vkPoolMutex{};
        VulkanDevice* device = nullptr;
        List<VkDescriptorPool> descriptorPools{};
        
        u32 initialSize = 128;
        u32 incrementSize = 128;
        
    };

} // namespace CE
