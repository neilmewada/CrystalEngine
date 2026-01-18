#pragma once

namespace CE::RHI
{
    class SwapChain;
    
    struct CORERHI_API RenderPassFrameAttachment
    {
    public:
        
        RenderPassFrameAttachment() = default;
        
        RenderPassFrameAttachment(RHI::Texture* texture)
            : texture(texture)
        {}
        
        RenderPassFrameAttachment(RHI::TextureView* textureView)
            : textureView(textureView)
        {}
        
        RenderPassFrameAttachment(RHI::SwapChain* swapChain)
            : swapChain(swapChain)
        {}
        
        RHI::Texture* GetTexture() const { return texture; }
        
        RHI::TextureView* GetTextureView() const { return textureView; }
        
        RHI::SwapChain* GetSwapChain() const { return swapChain; }
        
        bool IsValid() const
        {
            return texture != nullptr || textureView != nullptr || swapChain != nullptr;
        }
        
    private:
        
        RHI::Texture* texture = nullptr;
        RHI::TextureView* textureView = nullptr;
        RHI::SwapChain* swapChain = nullptr;
    };
    
} // namespace CE::RHI
