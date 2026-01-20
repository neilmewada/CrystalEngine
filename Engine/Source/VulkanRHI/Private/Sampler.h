#pragma once

namespace CE::Vulkan
{
    
    class VULKANRHI_API Sampler : public RHI::Sampler
    {
    public:
        Sampler(Device* device, const RHI::SamplerDescriptor& samplerDesc);
        virtual ~Sampler();
        
        virtual void* GetHandle() override
        {
            return sampler;
        }
        
        VkSampler GetVkHandle()
        {
            return sampler;
        }

		VkFilter ToVkFilter(RHI::FilterMode filterMode);
        
    private:
        
        Device* device = nullptr;
        VkSampler sampler{};
        SamplerBorderColor borderColor{};
    };

} // namespace CE
