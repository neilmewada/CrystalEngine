#pragma once

namespace CE::RHI
{
    class SwapChain;
    
    struct CORERHI_API RenderPassFrameAttachment
    {
    public:
        
        RenderPassFrameAttachment() = default;
        
        RenderPassFrameAttachment(RHI::Texture* texture)
        {
			for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                textures[i] = texture;
            }
        }

        RenderPassFrameAttachment(const StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount>& textures)
        {
            for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                this->textures[i] = textures[i];
            }
        }
        
        RenderPassFrameAttachment(int count, RHI::Texture** textures)
        {
            for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                this->textures[i] = textures[i];
            }
        }
        
        RenderPassFrameAttachment(RHI::TextureView* textureView)
        {
			for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                textureViews[i] = textureView;
            }
        }

		RenderPassFrameAttachment(const StaticArray<RHI::TextureView*, RHI::Limits::MaxSwapChainImageCount>& textureViews)
        {
            for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                this->textureViews[i] = textureViews[i];
            }
        }
        
        RenderPassFrameAttachment(int count, RHI::TextureView** textureViews)
        {
            for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                this->textureViews[i] = textureViews[i];
            }
        }
        
		//! @brief A SwapChain attachment. Note: imageIndex is only relevant for Vulkan SwapChains.
        RenderPassFrameAttachment(RHI::SwapChain* swapChain) : swapChain(swapChain)
        {}
        
        bool IsValid() const
        {
			if (swapChain != nullptr)
                return true;

			for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
            {
                if (textures[i] != nullptr || textureViews[i] != nullptr)
                    return true;
            }

            return false;
        }

        RHI::Texture* GetTexture(u32 index) const { return textures[index]; }

		RHI::TextureView* GetTextureView(u32 index) const { return textureViews[index]; }

		RHI::SwapChain* GetSwapChain() const { return swapChain; }
        
    private:

        StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount> textures;
        StaticArray<RHI::TextureView*, RHI::Limits::MaxSwapChainImageCount> textureViews;
        
        RHI::SwapChain* swapChain = nullptr;
    };
    
} // namespace CE::RHI
