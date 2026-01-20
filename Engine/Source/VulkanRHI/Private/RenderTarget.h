#pragma once

namespace CE::Vulkan
{
    
    class RenderTarget : public RHI::RenderTarget
    {
    public:
        RenderTarget(Device* device, const RHI::RenderTargetLayout& rtLayout);
        RenderTarget(Device* device, const VulkanRenderPass::Descriptor& rpDesc);
        RenderTarget(Device* device, VulkanRenderPass* renderPass);

        RenderTarget* Clone(const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) override;

        RenderTarget* Clone(RHI::MultisampleState msaa, const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) override;

        void GetAttachmentFormats(Array<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat, u32 subpassSelection) override;

        ~RenderTarget();

        u32 GetAttachmentCount() const;

        const VulkanRenderPass::AttachmentBinding& GetAttachment(u32 index);

        inline VulkanRenderPass* GetRenderPass() const { return renderPass; }

    private:

        Device* device = nullptr;
        VulkanRenderPass* renderPass = nullptr;

        friend class CommandList;
    };

} // namespace CE::Vulkan
