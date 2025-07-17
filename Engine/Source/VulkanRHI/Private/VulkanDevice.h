#pragma once

#include "CoreTypes.h"

#include "VulkanRHI.h"
#include "VulkanStructs.h"

#include <vulkan/vulkan.h>

namespace CE::Vulkan
{
    class CommandQueue;
    class CommandList;
    class SwapChain;
    class Texture;
	class DescriptorPool;
	class ShaderResourceManager;
	class CommandBufferAllocator;
	class RenderPassCache;
    class DeviceLimits;

    class VulkanDevice
    {
        CE_NO_COPY(VulkanDevice)
    public:
        VulkanDevice(VkInstance instance, VulkanRHI* vulkanRhi);
        ~VulkanDevice();

        INLINE bool IsInitialized() const
        {
            return isInitialized;
        }

        void Initialize();
        void PreShutdown();
        void Shutdown();

        // - Public API -

		void WaitUntilIdle();

        VkFormat FindSupportedFormat(const Array<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        VkSurfaceFormatKHR FindAutoColorFormat();

        bool CheckSurfaceFormatSupport(VkFormat format);

        s32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);

        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageViewType imageViewType, VkImageAspectFlags aspectFlags);

        int TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
            const VkImageSubresourceRange& subresource,
            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

        VkCommandBuffer BeginSingleUseCommandBuffer();
        void EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer);
        int SubmitAndWaitSingleUseCommandBuffer(VkCommandBuffer commandBuffer);

		VkCommandPool AllocateCommandBuffers(u32 count, VkCommandBuffer* outBuffers, RHI::CommandListType type, u32 queueFamilyIndex);

        // - Getters -

		Array<RHI::CommandQueue*> GetHardwareQueues(RHI::HardwareQueueClassMask queueMask);
		Array<RHI::CommandQueue*> AllocateHardwareQueues(const HashMap<RHI::HardwareQueueClass, int>& queueCountByClass);

		inline bool IsUnifiedMemoryArchitecture() const { return isUnifiedMemory; }

        inline bool SupportsHostCachedMemory() const { return supportsHostCachedMemory; }

        inline bool SupportsReBar() const { return supportsReBar; }

		inline CommandBufferAllocator* GetCommandAllocator() const { return commandAllocator; }

		INLINE bool IsOffscreenOnly() const { return !surfaceSupported; }

        INLINE VkCommandPool GetGraphicsCommandPool() const { return gfxCommandPool; }

        INLINE CommandQueue* GetGraphicsQueue() const { return primaryGraphicsQueue; }
        INLINE CommandQueue* GetPresentQueue() const { return presentQueue; }

        INLINE VkDevice GetHandle() const { return device; }
        INLINE VkPhysicalDevice GetPhysicalHandle() const { return gpu; }

        INLINE const SurfaceSupportInfo& GetSurfaceSupportInfo() const { return surfaceSupport; }

		INLINE bool IsDeviceExtensionSupported(const String& name) const
		{
			return supportedDeviceExtensions.Exists(name);
		}

        DescriptorPool* GetDescriptorPool();

		INLINE const VkPhysicalDeviceLimits& GetDeviceLimits() const
		{
			return gpuProperties.limits;
		}

		INLINE ShaderResourceManager* GetShaderResourceManager() const
		{
			return srgManager;
		}

		INLINE const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const
		{
			return memoryProperties;
		}

		INLINE RenderPassCache* GetRenderPassCache() const { return renderPassCache; }

        const Array<RHI::Format>& GetAvailableDepthStencilFormats();
        const Array<RHI::Format>& GetAvailableDepthOnlyFormats();

        void SetObjectDebugName(uint64_t objectHandle, VkObjectType objectType, const char* objectName);

    protected:

    private:

        void SelectGpu();
        void InitGpu();
        bool QueryGpu(u32 gpuIndex);

		void FetchQueues(VkPhysicalDevice gpu);

        VulkanQueueFamilies GetQueueFamilies(VkPhysicalDevice gpu);
        VkDeviceSize GetPhysicalDeviceLocalMemory(VkPhysicalDevice gpu);
        SurfaceSupportInfo FetchSurfaceSupportInfo(VkPhysicalDevice gpu);

        bool isInitialized = false;
		bool isUnifiedMemory = false;
        bool supportsReBar = false;
        bool supportsHostCachedMemory = false;

		/// @brief Vulkan can be initialized without any Native windows (i.e. offscreen mode).
		/// Potentially useful for command line applications/toos.
		bool surfaceSupported = false;

        VkInstance instance = nullptr;
        VulkanRHI* vulkanRhi = nullptr;
        DeviceLimits* deviceLimits = nullptr;

		VkPhysicalDeviceMemoryProperties memoryProperties{};
		HashSet<int> ignoredHeapIndices{};

		Array<Name> supportedDeviceExtensions{};

        VkPhysicalDevice gpu = nullptr;
        VkPhysicalDeviceProperties gpuProperties{};
		Array<VkQueueFamilyProperties> queueFamilyProperties{};
        Array<RHI::Format> availableDepthStencilFormats{};
        Array<RHI::Format> availableDepthOnlyFormats{};

        GpuMetaData gpuMetaData{};

        VkDevice device = nullptr;
        //VulkanQueueFamilies queueFamilies{};
        SurfaceSupportInfo surfaceSupport{};

        VkSurfaceKHR testSurface = nullptr;

		Array<CommandQueue*> queues{};
        Array<CommandQueue*> presentQueues{};
		HashMap<int, Array<CommandQueue*>> queuesByFamily{};

		CommandBufferAllocator* commandAllocator = nullptr;
        CommandQueue* primaryGraphicsQueue = nullptr;
        CommandQueue* primaryTransferQueue = nullptr;
        CommandQueue* presentQueue = nullptr;

		RenderPassCache* renderPassCache = nullptr;
		ShaderResourceManager* srgManager = nullptr;
        VkCommandPool gfxCommandPool = nullptr;

        HashMap<u32, VkCommandPool> queueFamilyToCmdPool{};

		SharedMutex mainThreadMutex{};

		//DescriptorPool* descriptorPool = nullptr;
        ThreadLocalContext<DescriptorPool> threadedDescriptorPool;

        // Debug functions
        PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectName = nullptr;
        
        friend class FrameGraphCompiler;
        friend class VulkanRHI;
        friend class CE::Vulkan::CommandList;
        friend class DeviceLimits;
    };
    
} // namespace CE
