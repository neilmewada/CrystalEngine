#pragma once

namespace CE::Metal
{
    
    class SwapChain : public RHI::SwapChain
    {
    public:
        SwapChain(Device* device, PlatformWindow* window, const RHI::SwapChainDescriptor& desc);
        
        PlatformWindow* GetNativeWindow() override { return window; }
        
        void Rebuild() override;
        
    private:
        
        Device* device = nullptr;
        PlatformWindow* window = nullptr;

        RHI::SwapChainDescriptor desc{};
        
    private:
        
        CAMetalLayer* metalLayer = nil;
        
    };

} // namespace CE::Metal
