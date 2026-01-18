#pragma once

namespace CE::Metal
{

    class RenderPass : public RHI::RenderPass
    {
    public:
        RenderPass(Device* device, const RHI::RenderPassLayout& rpLayout);
        virtual ~RenderPass();
        
        RenderPass* Clone(const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) override;
        
        RenderPass* Clone(MultisampleState msaa, const Array<RHI::Format>& newColorFormats, RHI::Format depthStencilFormat, u32 subpassSelection) override;
        
        void GetAttachmentFormats(Array<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat, u32 subpassSelection) override;
        
        int GetSubpassCount() const { return (int)[renderPassDescArray count]; }
        
        MTLRenderPassDescriptor* GetSubpass(int index)
        {
            if (index < 0 || index >= GetSubpassCount())
                return nil;
            return (MTLRenderPassDescriptor*)[renderPassDescArray objectAtIndex:index];
        }
        
    private:
        
        Device* device = nullptr;
        RHI::RenderPassLayout rpLayout{};
        
        NSMutableArray* renderPassDescArray = nil;
    };
    
} // namespace CE::Metal
