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
        
        RenderPassFrameAttachment(RHI::SwapChain* swapChain, u32 imageIndex)
            : swapChain(swapChain), imageIndex(imageIndex)
        {}
        
        RHI::Texture* GetTexture() const { return texture; }
        
        RHI::TextureView* GetTextureView() const { return textureView; }
        
        RHI::SwapChain* GetSwapChain() const { return swapChain; }

        u32 GetImageIndex() const { return imageIndex; }
        
        bool IsValid() const
        {
            return texture != nullptr || textureView != nullptr || swapChain != nullptr;
        }
        
    private:
        
        RHI::Texture* texture = nullptr;
        RHI::TextureView* textureView = nullptr;
        RHI::SwapChain* swapChain = nullptr;
        u32 imageIndex = 0;
    };
    
} // namespace CE::RHI
