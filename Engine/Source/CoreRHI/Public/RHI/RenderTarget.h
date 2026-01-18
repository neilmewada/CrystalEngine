#pragma once

namespace CE::RHI
{

    struct CE_DEPRECATED("RHI::RenderAttachmentLayout is deprecated. Please use RHI::RenderPassAttachmentLayout instead.") RenderAttachmentLayout;

    struct RenderAttachmentLayout
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

    struct CE_DEPRECATED("RHI::RenderTargetSubpassLayout is deprecated. Please use RHI::RenderPassSubpassLayout instead.") RenderTargetSubpassLayout;

    struct RenderTargetSubpassLayout
    {
        FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount> colorAttachments{};
        FixedArray<u32, 1> depthStencilAttachment{};
        FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount> resolveAttachments{};
        FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount> subpassInputAttachments{};
    };

    struct CE_DEPRECATED("RHI::RenderTargetLayout is deprecated. Please use RHI::RenderPassLayout instead.") RenderTargetLayout;

    struct RenderTargetLayout
    {
        FixedArray<RenderAttachmentLayout, RHI::Limits::Pipeline::MaxRenderAttachmentCount> attachmentLayouts{};
        FixedArray<RenderTargetSubpassLayout, RHI::Limits::Pipeline::MaxSubPassCount> subpasses{};
    };

    class CE_DEPRECATED("RHI::RenderTarget is deprecated. Please use RHI::RenderPass instead!") RenderTarget;

    class CORERHI_API RenderTarget : RHI::RHIResource
    {
    public:

        virtual ~RenderTarget() {}

        //! @brief Clones the RenderTarget with modified attachment formats based on the given subpass number.
        //! @param newColorFormats: Array of formats to use for color attachments of subpass 'subpassSelection'
        //! @param depthStencilFormat: Depth stencil format to use
        //! @param subpassSelection: The subpass to use for consideration. The cloned RenderTarget will contain all the subpass regardless of subpassSelection.
        //! @return 
        virtual RenderTarget* Clone(const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) = 0;


        virtual RenderTarget* Clone(MultisampleState msaa, const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) = 0;

        //! @brief Returns the formats of color & depth-stencil attachments at the given subpass number.
        virtual void GetAttachmentFormats(Array<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat, u32 subpassSelection) = 0;

    protected:
        RenderTarget() : RHI::RHIResource(RHI::ResourceType::RenderTarget)
        {}
        
    };
    
} // namespace CE::RHI
