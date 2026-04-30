#pragma once

namespace CE::Vulkan
{

    class VULKANRHI_API TransientAttachmentPool : public RHI::TransientAttachmentPool
    {
    public:
		
        TransientAttachmentPool(VulkanDevice* device);

    private:

        VulkanDevice* device = nullptr;
    };
    
} // namespace CE::Vulkan
