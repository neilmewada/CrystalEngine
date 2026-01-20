#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	RenderPass::RenderPass(Device* device, const RHI::RenderPassLayout& rpLayout) : device(device), RHI::RenderPass(rpLayout)
	{
		VulkanRenderPass::Descriptor desc{};
		desc.name = "RenderTarget";
		VulkanRenderPass::BuildDescriptor(rpLayout, desc);
		renderPass = device->GetRenderPassCache()->FindOrCreate(desc);
	}

	RenderPass::~RenderPass()
	{
		// Never destroy a render pass manually, it is cached and destroyed automatically when RHI is shut down.
	}

	RHI::RenderPass* RenderPass::Clone(const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection)
	{
        RHI::RenderPassLayout newRPLayout = rpLayout;

        const auto& subpass = newRPLayout.subpasses[subpassSelection];

        for (int i = 0; i < subpass.colorAttachments.GetSize(); i++)
        {
            if (i >= newColorFormats.GetSize())
                break;
            if (newColorFormats[i] == RHI::Format::Undefined)
                continue;

            u32 attachmentIndex = subpass.colorAttachments[i];

            RHI::RenderPassAttachmentLayout& attachment = newRPLayout.attachmentLayouts[attachmentIndex];
            attachment.format = newColorFormats[i];
        }

        if (subpass.depthStencilAttachment.GetSize() > 0 && depthStencilFormat != RHI::Format::Undefined)
        {
            u32 attachmentIndex = subpass.depthStencilAttachment[0];
            newRPLayout.attachmentLayouts[attachmentIndex].format = depthStencilFormat;
        }

        return new RenderPass(device, newRPLayout);
	}

	RHI::RenderPass* RenderPass::Clone(MultisampleState msaa, const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection)
	{
        RHI::RenderPassLayout newRPLayout = rpLayout;

        const auto& subpass = newRPLayout.subpasses[subpassSelection];

        for (int i = 0; i < subpass.colorAttachments.GetSize(); i++)
        {
            if (i >= newColorFormats.GetSize())
                break;
            if (newColorFormats[i] == RHI::Format::Undefined)
                continue;

            u32 attachmentIndex = subpass.colorAttachments[i];
            RHI::RenderPassAttachmentLayout& attachment = newRPLayout.attachmentLayouts[attachmentIndex];

            attachment.format = newColorFormats[i];
            attachment.multisampleState = msaa;
        }

        if (subpass.depthStencilAttachment.GetSize() > 0 && depthStencilFormat != RHI::Format::Undefined)
        {
            u32 attachmentIndex = subpass.depthStencilAttachment[0];
            newRPLayout.attachmentLayouts[attachmentIndex].format = depthStencilFormat;
            newRPLayout.attachmentLayouts[attachmentIndex].multisampleState = msaa;
        }

        return new RenderPass(device, newRPLayout);
	}

	void RenderPass::GetAttachmentFormats(Array<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat, u32 subpassSelection)
	{
        outColorFormats.Clear();
        outDepthStencilFormat = RHI::Format::Undefined;

        const auto& subpass = rpLayout.subpasses[subpassSelection];

        for (int i = 0; i < subpass.colorAttachments.GetSize(); i++)
        {
            u32 attachmentIndex = subpass.colorAttachments[i];
            outColorFormats.Add(rpLayout.attachmentLayouts[attachmentIndex].format);
        }

        if (subpass.depthStencilAttachment.GetSize() > 0)
        {
            u32 attachmentIndex = subpass.depthStencilAttachment[0];
            outDepthStencilFormat = rpLayout.attachmentLayouts[attachmentIndex].format;
        }
	}

} // namespace CE::Vulkan
