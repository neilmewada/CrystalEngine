#pragma once

namespace CE::Vulkan
{
    class ShaderResourceManager;
    class CommandList;

    class DescriptorSet
    {
    public:
        DescriptorSet(Device* device, VkDescriptorSetLayout setLayout, const RHI::ShaderResourceGroupLayout& srgLayout, const String& name = "");
        DescriptorSet(Device* device, VkDescriptorSetLayout setLayout, const RHI::ShaderResourceGroupLayout& srgLayout, u32 arrayCount, const String& name = "");

        virtual ~DescriptorSet();

        inline VkDescriptorSet GetHandle() const { return descriptorSet; }

    private:

        Device* device = nullptr;

        RHI::ShaderResourceGroupLayout srgLayout{};
        int setNumber = -1;

        VkDescriptorSet descriptorSet = nullptr;
        VkDescriptorPool descriptorPool = nullptr;
        ShaderResourceManager* srgManager = nullptr;
        DescriptorPool* pool = nullptr;

        Atomic<int> usageCount = 0;
        int frameCounter = 0;

        bool failed = false;

        friend class ShaderResourceManager;
        friend class ShaderResourceGroup;
        friend class CE::Vulkan::CommandList;
    };
    
} // namespace CE::Vulkan
