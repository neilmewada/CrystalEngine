#pragma once

namespace CE::RHI
{
    
    struct RenderPassAttachmentLayout
    {
        AttachmentID attachmentId{};
        ScopeAttachmentUsage attachmentUsage{};
        RHI::Format format{};
        RHI::MultisampleState multisampleState{};

        AttachmentLoadAction loadAction{};
        AttachmentStoreAction storeAction{};

        AttachmentLoadAction loadActionStencil{};
        AttachmentStoreAction storeActionStencil{};
    };

    struct RenderPassSubpassLayout
    {
        FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount> colorAttachments{};
        FixedArray<u32, 1> depthStencilAttachment{};
        FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount> resolveAttachments{};
        FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount> subpassInputAttachments{};
    };

    struct RenderPassLayout
    {
        FixedArray<RenderPassAttachmentLayout, RHI::Limits::Pipeline::MaxRenderAttachmentCount> attachmentLayouts{};
        FixedArray<RenderPassSubpassLayout, RHI::Limits::Pipeline::MaxSubPassCount> subpasses{};
    };

    class CORERHI_API RenderPass : RHI::RHIResource
    {
    public:

        virtual ~RenderPass() {}

        //! @brief Clones the RenderPass with modified attachment formats based on the given subpass number.
        //! @param newColorFormats: Array of formats to use for color attachments of subpass 'subpassSelection'
        //! @param depthStencilFormat: Depth stencil format to use
        //! @param subpassSelection: The subpass to use for consideration. The cloned RenderPass will contain all the subpass regardless of subpassSelection.
        //! @return
        virtual RenderPass* Clone(const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) = 0;

        //! @brief Clones the RenderPass with modified attachment formats, msaa on the given subpass number.
        //! @param msaa: MSAA to use at given subpassSelection.
        //! @param newColorFormats: Array of formats to use for color attachments of subpass 'subpassSelection'
        //! @param depthStencilFormat: Depth stencil format to use
        //! @param subpassSelection: The subpass to use for consideration. The cloned RenderPass will contain all the subpass regardless of subpassSelection.
        virtual RenderPass* Clone(MultisampleState msaa, const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) = 0;

        //! @brief Returns the formats of color & depth-stencil attachments at the given subpass number.
        virtual void GetAttachmentFormats(Array<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat, u32 subpassSelection) = 0;

    protected:
        RenderPass() : RHI::RHIResource(RHI::ResourceType::RenderPass)
        {}
        
    };
    
} // namespace CE::RHI
