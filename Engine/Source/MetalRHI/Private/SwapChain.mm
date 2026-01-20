#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{

    SwapChain::SwapChain(Device* device, PlatformWindow* window, const RHI::SwapChainDescriptor& desc)
        : device(device), window(window), desc(desc)
    {
        this->desc.preferredFormats.AddRange({ RHI::Format::R8G8B8A8_UNORM, RHI::Format::B8G8R8A8_UNORM });
        this->preferredWidth = desc.preferredWidth;
        this->preferredHeight = desc.preferredHeight;
        this->swapChainColorFormat = this->desc.preferredFormats[0];
        this->frameBufferOnly = desc.frameBufferOnly;
        
        metalLayer = MetalPlatform::GetCAMetalLayer(window);
        
        metalLayer.device = device->GetHandle();
        metalLayer.maximumDrawableCount = desc.imageCount;
        metalLayer.pixelFormat = ToMtlFormat(this->swapChainColorFormat);
        metalLayer.framebufferOnly = frameBufferOnly;
        
        window->GetDrawableWindowSize(&width, &height);
    }

    void SwapChain::Rebuild()
    {
        // Do nothing
    }
    
    bool SwapChain::AcquireNextImage()
    {
        curDrawable = [metalLayer nextDrawable];
        
        return curDrawable != nil;
    }
    
} // namespace CE::Metal
