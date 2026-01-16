#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    SwapChain::SwapChain(Device* device, PlatformWindow* window, const RHI::SwapChainDescriptor& desc)
        : device(device), window(window), desc(desc)
    {
        this->desc.preferredFormats.AddRange({ RHI::Format::R8G8B8A8_UNORM, RHI::Format::B8G8R8A8_UNORM });
        this->preferredWidth = desc.preferredWidth;
        this->preferredHeight = desc.preferredHeight;
        
        window->GetDrawableWindowSize(&width, &height);
        
        metalLayer = MetalPlatform::GetCAMetalLayer(window);
        
        String::IsAlphabet('a');
    }

    void SwapChain::Rebuild()
    {
        
    }
    
} // namespace CE::Metal
