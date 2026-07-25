#pragma once

#include "VulkanRHIPrivate.h"

namespace CE {
	class PlatformWindow;
}

namespace CE::Vulkan
{
    class VulkanFrameBuffer;

    /*class Viewport : public RHI::Viewport
    {
    public:
        Viewport(VulkanRHI* vulkanRHI, Device* device, PlatformWindow* windowHandle,
            u32 width, u32 height,
            bool isFullscreen, const RHI::RenderTargetLayout& rtLayout);
        virtual ~Viewport();

        virtual RHI::RenderTarget* GetRenderTarget() override;

        virtual void Rebuild() override;
        
        virtual void OnResize() override
        {
            Rebuild();
        }

        // - Setters -

        virtual void SetClearColor(const Color& color) override;

        // - Getters -

        SwapChain* GetSwapChain() const { return swapChain; }

        PlatformWindow* GetWindowHandle() const { return windowHandle; }

        u32 GetBackBufferCount();
        u32 GetSimultaneousFrameDrawCount();
        u32 GetWidth();
        u32 GetHeight();

    protected:

        void CreateFrameBuffers();
        void CreateSyncObjects();
        
        void OnWindowResized(PlatformWindow* window, u32 width, u32 height);

    private:
        Color clearColor{};
        bool isFullscreen = false;

        PlatformWindow* windowHandle = nullptr;
        VulkanRHI* vulkanRHI = nullptr;
        Device* device = nullptr;
        RenderTarget* renderTarget = nullptr;
        SwapChain* swapChain = nullptr;

        Array<VulkanFrameBuffer*> frameBuffers{};

        // Sync Objects (Per simultaneous frame)
        // Count = number of frames that are rendered simultaneously
        List<VkSemaphore> imageAcquiredSemaphore{};

        u32 currentDrawFrameIndex = 0;
        u32 currentImageIndex = 0;
        
        DelegateHandle windowResizeDelegateHandle = 0;

        friend struct VulkanRenderTargetLayout;
        friend class GraphicsCommandList;
        friend class VulkanRHI;
    };*/
    
} // namespace CE
