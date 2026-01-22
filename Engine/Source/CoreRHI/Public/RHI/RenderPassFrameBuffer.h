#pragma once

namespace CE::RHI
{
    
    struct RenderPassFrameBufferDescriptor
    {
        RenderPass* renderPass = nullptr;
        Array<RenderPassFrameAttachment> attachments{};
    };

    /**
     * \brief A VkFramebuffer object in case of Vulkan.
     */
    class CORERHI_API RenderPassFrameBuffer : public RHI::RHIResource
    {
    public:
        virtual ~RenderPassFrameBuffer() = default;

        RenderPass* GetRenderPass() const { return desc.renderPass; }
        
        u32 GetAttachmentCount() const { return desc.attachments.GetSize(); }
        
        const RenderPassFrameAttachment& GetAttachment(u32 index) const { return desc.attachments[index]; }
        
        virtual u32 GetWidth() = 0;
        virtual u32 GetHeight() = 0;
        
    protected:

        RenderPassFrameBuffer(const RHI::RenderPassFrameBufferDescriptor& desc) : RHIResource(RHI::ResourceType::RenderPassFrameBuffer), desc(desc)
        {}
        
        RHI::RenderPassFrameBufferDescriptor desc;
    };

} // namespace CE::RHI
