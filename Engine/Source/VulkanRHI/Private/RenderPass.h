#pragma once

namespace CE::Vulkan
{

	class VULKANRHI_API RenderPass : public RHI::RenderPass
    {
    public:
        RenderPass(Device* device, const RHI::RenderPassLayout& rpLayout);

        ~RenderPass();

        RHI::RenderPass* Clone(const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat,
	        u32 subpassSelection) override;

        RHI::RenderPass* Clone(MultisampleState msaa, const Array<RHI::Format>& newColorFormats,
	        RHI::Format depthStencilFormat, u32 subpassSelection) override;

        void GetAttachmentFormats(Array<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat,
	        u32 subpassSelection) override;

        VkRenderPass GetHandle() const { return renderPass->GetHandle(); }

		VulkanRenderPass* GetVulkanRenderPass() const { return renderPass; }

    private:

		Device* device = nullptr;

        VulkanRenderPass* renderPass = nullptr;
    };
    
} // namespace CE::Vulkan
