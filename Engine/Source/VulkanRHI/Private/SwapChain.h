#pragma once

#include "VulkanRHIPrivate.h"

#include "vulkan/vulkan.h"

namespace CE {
	class PlatformWindow;
}

namespace CE::Vulkan
{
    class Device;
    class Texture;

    struct VulkanSwapChainImage
    {
        VkImage image = nullptr;
        VkImageView imageView = nullptr;
    };

	class SwapChain : public RHI::SwapChain
	{
	public:
		SwapChain(VulkanRHI* rhi, Device* device, PlatformWindow* window, const RHI::SwapChainDescriptor& desc);

		virtual ~SwapChain();

		void RebuildSwapChain();

		bool AcquireNextImage() override;

		inline VkSwapchainKHR GetHandle() const { return swapChain; }

		PlatformWindow* GetNativeWindow() override { return window; }

		void Rebuild() override
		{
			RebuildSwapChain();
		}
        
        Vulkan::Texture* GetCurrentImage() { return images[currentImageIndex]; }

		u32 GetCurrentImageIndex() const { return currentImageIndex; }

		u32 GetImageCount() const { return images.GetSize(); }

		Vulkan::Texture* GetImage(u32 imageIndex) const { return images[imageIndex]; }

		inline s64 GetSwapChainId() const { return swapChainId; }

		u32 GetWidth() override { return width; }
		u32 GetHeight() override { return height; }

	protected:

		void OnWindowResized(PlatformWindow* window, u32 newDrawWidth, u32 newDrawHeight);

		void Create();

	protected:
		VulkanRHI* rhi = nullptr;
		Device* device = nullptr;
		PlatformWindow* window = nullptr;

		RHI::SwapChainDescriptor desc{};

		u32 width = 0;
		u32 height = 0;
        
        Array<Vulkan::Texture*> images{};
		u32 currentImageIndex = 0;
		u32 currentImageAcquiredSemaphoreIndex = 0;
		bool shouldRebuild = false;
		s64 swapChainId = 0;
		
		StaticArray<VkImageLayout, RHI::Limits::MaxSwapChainImageCount> swapChainInitialImageLayouts{};
		StaticArray<VkSemaphore, RHI::Limits::MaxSwapChainImageCount> imageAcquiredSemaphores{};
		StaticArray<VkSemaphore, RHI::Limits::MaxSwapChainImageCount> renderFinishedSemaphores{};

		List<VkSurfaceFormatKHR> surfaceFormats{};
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		List<VkPresentModeKHR> presentationModes{};

		//! @brief We do not support triple buffering, etc for the time being.
		u32 simultaneousFramesInFlight = 1;

		VkSwapchainKHR swapChain = nullptr;
		VkSurfaceKHR surface = nullptr;

		VkPresentModeKHR presentMode{};
		VkSurfaceFormatKHR swapChainSurfaceFormat{};

		DelegateHandle windowResizeCallback = 0;

		friend class FrameGraphExecuter;
		friend class CommandQueue;
	};

} // namespace CE
