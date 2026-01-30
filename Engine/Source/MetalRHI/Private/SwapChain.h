#pragma once

namespace CE::Metal
{
    
    class SwapChain : public RHI::SwapChain
    {
    public:
        SwapChain(Device* device, PlatformWindow* window, const RHI::SwapChainDescriptor& desc);
        
        PlatformWindow* GetNativeWindow() override { return window; }
        
        void Rebuild() override;
        
        CAMetalLayer* GetMetalLayer() const { return metalLayer; }
        
        id<CAMetalDrawable> GetCurrentDrawable() const { return curDrawable; }
        
        bool AcquireNextImage() override;
        
        u32 GetWidth() override;
        
        u32 GetHeight() override;
        
    private:
        
        Device* device = nullptr;
        PlatformWindow* window = nullptr;

        RHI::SwapChainDescriptor desc{};
        
    private:
        
        CAMetalLayer* metalLayer = nil;
        id<CAMetalDrawable> curDrawable = nil;
        
        friend class Metal::CommandQueue;
    };

} // namespace CE::Metal
