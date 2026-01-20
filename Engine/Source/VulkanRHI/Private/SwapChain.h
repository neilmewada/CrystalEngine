#pragma once

#include "VulkanRHIPrivate.h"

#include "vulkan/vulkan.h"

namespace CE {
	class PlatformWindow;
}

namespace CE::Vulkan
{
    class VulkanDevice;
    class Texture;

    struct VulkanSwapChainImage
    {
        VkImage image = nullptr;
        VkImageView imageView = nullptr;
    };

	class SwapChain : public RHI::SwapChain
	{
	public:
		SwapChain(VulkanRHI* rhi, VulkanDevice* device, PlatformWindow* window, const RHI::SwapChainDescriptor& desc);

		virtual ~SwapChain();

		void RebuildSwapChain();

		inline VkSwapchainKHR GetHandle() const { return swapChain; }

		PlatformWindow* GetNativeWindow() override { return window; }

		void Rebuild() override
		{
			RebuildSwapChain();
		}

	protected:

		void OnWindowResized(PlatformWindow* window, u32 newDrawWidth, u32 newDrawHeight);

		void Create();

		VulkanRHI* rhi = nullptr;
		VulkanDevice* device = nullptr;
		PlatformWindow* window = nullptr;

		RHI::SwapChainDescriptor desc{};

		//! Used to 
		StaticArray<VkImageLayout, RHI::Limits::MaxSwapChainImageCount> swapChainInitialImageLayouts{};

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
	};

} // namespace CE
