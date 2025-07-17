
#include "VulkanRHIPrivate.h"
#include "PAL/Common/VulkanPlatform.h"

#define VMA_IMPLEMENTATION
//#include <vma/vk_mem_alloc.h>

namespace CE::Vulkan
{
	static void InitDescriptorPool(DescriptorPool& pool)
	{
		pool.Init(((VulkanRHI*)gDynamicRHI)->GetDevice(), 64, 64);
	}

	VulkanDevice::VulkanDevice(VkInstance instance, VulkanRHI* vulkanRhi)
		: instance(instance), vulkanRhi(vulkanRhi), threadedDescriptorPool(ThreadLocalContext<DescriptorPool>(InitDescriptorPool))
	{
		
	}

	VulkanDevice::~VulkanDevice()
	{

	}

	void VulkanDevice::Initialize()
	{
		// Check if a main window exists, otherwise initialize in offscreen mode
		auto mainWindow = VulkanPlatform::GetMainPlatformWindow();
		surfaceSupported = true;

		if (mainWindow != nullptr)
			testSurface = VulkanPlatform::CreateSurface(instance, mainWindow);
		else
			surfaceSupported = false; // Initialize vulkan in offscreen mode

		setDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

		SelectGpu();
		InitGpu();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = primaryGraphicsQueue->GetFamilyIndex();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(device, &poolInfo, VULKAN_CPU_ALLOCATOR, &gfxCommandPool) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create Graphics Command Pool");
			return;
		}

		if (testSurface != nullptr)
			VulkanPlatform::DestroySurface(instance, testSurface);
		testSurface = nullptr;
		
		//descriptorPool = new DescriptorPool(this);

		srgManager = new ShaderResourceManager(this);

		commandAllocator = new CommandBufferAllocator(this);

		renderPassCache = new RenderPassCache(this);

		isInitialized = true;

		CE_LOG(Info, All, "Vulkan device initialized");
	}

	void VulkanDevice::PreShutdown()
	{
		isInitialized = false;

		delete renderPassCache;
		renderPassCache = nullptr;

		delete commandAllocator;
		commandAllocator = nullptr;

		delete srgManager;
		srgManager = nullptr;

		//delete descriptorPool; descriptorPool = nullptr;
		threadedDescriptorPool.Clear();

		for (auto queue : queues)
		{
			delete queue;
		}
		queues.Clear();
	}

	void VulkanDevice::Shutdown()
	{
		delete deviceLimits; deviceLimits = nullptr;

		vkDestroyCommandPool(device, gfxCommandPool, VULKAN_CPU_ALLOCATOR);
		gfxCommandPool = nullptr;

		// Command Pools
		for (const auto& pair : queueFamilyToCmdPool)
		{
			vkDestroyCommandPool(device, pair.second, VULKAN_CPU_ALLOCATOR);
		}
		queueFamilyToCmdPool.Clear();

		vkDestroyDevice(device, VULKAN_CPU_ALLOCATOR);
		device = nullptr;

		CE_LOG(Info, All, "Vulkan device shutdown");
	}

	DescriptorPool* VulkanDevice::GetDescriptorPool()
	{
		return &threadedDescriptorPool.GetStorage();
		//return descriptorPool;
	}

	const Array<RHI::Format>& VulkanDevice::GetAvailableDepthStencilFormats()
	{
		if (availableDepthStencilFormats.IsEmpty())
		{
			StaticArray<VkFormat, 3> formats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT };

			for (VkFormat format : formats)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

				if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					availableDepthStencilFormats.Add(VkFormatToRHIFormat(format));
				}
			}
		}

		return availableDepthStencilFormats;
	}

	const Array<RHI::Format>& VulkanDevice::GetAvailableDepthOnlyFormats()
	{
		if (availableDepthOnlyFormats.IsEmpty())
		{
			StaticArray<VkFormat, 2> formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM };

			for (VkFormat format : formats)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

				if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					availableDepthOnlyFormats.Add(VkFormatToRHIFormat(format));
				}
			}
		}

		return availableDepthOnlyFormats;
	}

	void VulkanDevice::SetObjectDebugName(uint64_t objectHandle, VkObjectType objectType, const char* objectName)
	{
		if (setDebugUtilsObjectName)
		{
			VkDebugUtilsObjectNameInfoEXT info{};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.objectType = objectType;
			info.objectHandle = objectHandle;
			info.pObjectName = objectName;
			setDebugUtilsObjectName(device, &info);
		}

	}

	void VulkanDevice::SelectGpu()
	{
		// Fetch all available physical devices
		u32 physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices{ physicalDeviceCount };
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		if (physicalDeviceCount == 1)
		{
			this->gpu = physicalDevices[0];
			vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
			gpuMetaData.localMemorySize = GetPhysicalDeviceLocalMemory(gpu);
			CE_LOG(Info, All, "Selected {} as the target GPU.", gpuProperties.deviceName);
			return;
		}

		if (physicalDeviceCount == 0)
		{
			CE_LOG(Error, All, "Failed to select a target GPU. No Vulkan supported GPU found!");
			return;
		}

		VkDeviceSize selectedDeviceMemorySize = 0;
		u32 selectedGpuIndex = 0;

		for (int i = 0; i < physicalDevices.size(); ++i)
		{
			const auto& physicalDevice = physicalDevices[i];

			VkPhysicalDeviceProperties props = {};
			vkGetPhysicalDeviceProperties(physicalDevice, &props);

			VkPhysicalDeviceFeatures features = {};
			vkGetPhysicalDeviceFeatures(physicalDevice, &features);

			auto localMemorySize = GetPhysicalDeviceLocalMemory(physicalDevice);
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
				localMemorySize > selectedDeviceMemorySize)
			{
				if (QueryGpu(i))
				{
					selectedGpuIndex = i;
					selectedDeviceMemorySize = localMemorySize;
				}
			}
		}

		this->gpu = physicalDevices[selectedGpuIndex];
		vkGetPhysicalDeviceProperties(this->gpu, &gpuProperties);

		CE_LOG(Info, All, "Selected {} as the target GPU", gpuProperties.deviceName);
	}

	void VulkanDevice::InitGpu()
	{
		// Fetch memory properties
		vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);

		isUnifiedMemory = true;
		int dedicatedHeapIndex = -1;
		u64 dedicatedHeapSize = 0;

		for (int i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			if ((memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) && memoryProperties.memoryHeaps[i].size > dedicatedHeapSize)
			{
				dedicatedHeapSize = memoryProperties.memoryHeaps[i].size;
				dedicatedHeapIndex = i;
			}

			if (memoryProperties.memoryHeaps[i].flags != VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				isUnifiedMemory = false;
			}
		}

		supportsHostCachedMemory = false;
		supportsReBar = false;

		for (int i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			VkMemoryType memType = memoryProperties.memoryTypes[i];
			VkMemoryPropertyFlags rebarFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			if ((memType.propertyFlags & rebarFlags) == rebarFlags && memType.heapIndex == dedicatedHeapIndex)
			{
				supportsReBar = true;
			}

			if (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			{
				supportsHostCachedMemory = true;
			}
		}

		// TODO: Temporary code
		// Using ReBar and skipping staging buffers leads to crash ONLY when using Nsight frame debugger.
		// Should be enabled for production code.
#if CE_DEBUG
		supportsReBar = false;
#endif

		vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
		gpuMetaData.localMemorySize = GetPhysicalDeviceLocalMemory(gpu);

		// Store Queue Families & Surface Support Info for later use
		//queueFamilies = GetQueueFamilies(gpu);
		surfaceSupport = FetchSurfaceSupportInfo(gpu);
		
		// Fetch Queue properties
		u32 queuePropertyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queuePropertyCount, nullptr);
		queueFamilyProperties.Clear();
		queueFamilyProperties.Resize(queuePropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queuePropertyCount, queueFamilyProperties.GetData());

		Array<VkDeviceQueueCreateInfo> queueCreateInfos{};
		queueCreateInfos.Resize(queueFamilyProperties.GetSize());

		float queuePriority = 1.0f;
		Array<float> queuePriorities{};
		int totalQueues = 0;

		for (int familyIdx = 0; familyIdx < queueFamilyProperties.GetSize(); familyIdx++)
		{
			totalQueues += queueFamilyProperties[familyIdx].queueCount;
		}

		queuePriorities.Resize(totalQueues);

		int baseIdx = 0;

		for (int familyIdx = 0; familyIdx < queueFamilyProperties.GetSize(); familyIdx++)
		{
			int queueCount = queueFamilyProperties[familyIdx].queueCount;

			for (int i = 0; i < queueCount; i++)
				queuePriorities[baseIdx + i] = 1.0f;

			VkDeviceQueueCreateInfo queueCI = {};
			queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCI.queueFamilyIndex = familyIdx;
			queueCI.queueCount = queueCount;
			queueCI.pQueuePriorities = &queuePriorities[baseIdx];

			queueCreateInfos[familyIdx] = queueCI;

			baseIdx += queueCount;
		}

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.GetSize());
		deviceCI.pQueueCreateInfos = queueCreateInfos.GetData();

		auto deviceExtensionNames = Array<const char*>();

		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &deviceExtensionCount, nullptr);
		auto deviceExtensionProperties = Array<VkExtensionProperties>(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &deviceExtensionCount, deviceExtensionProperties.GetData());

		supportedDeviceExtensions.Clear();

		for (int i = 0; i < deviceExtensionCount; ++i)
		{
			// Required rule by Vulkan Specs, especially on Apple platform.
			if (strcmp(deviceExtensionProperties[i].extensionName, "VK_KHR_portability_subset") == 0)
			{
				deviceExtensionNames.Add(deviceExtensionProperties[i].extensionName);
			}

			supportedDeviceExtensions.Add(deviceExtensionProperties[i].extensionName);

			if (strcmp(deviceExtensionProperties[i].extensionName, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
			{
				deviceExtensionNames.Add(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
			}

			if (strcmp(deviceExtensionProperties[i].extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
			{
				deviceExtensionNames.Add(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
			}

			if (strcmp(deviceExtensionProperties[i].extensionName, VK_KHR_MAINTENANCE3_EXTENSION_NAME) == 0)
			{
				deviceExtensionNames.Add(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
			}

#if CE_DEBUG && PLATFORM_DESKTOP
			// Debugging Extensions
			if (strcmp(deviceExtensionProperties[i].extensionName, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME) == 0)
			{
				deviceExtensionNames.Add(deviceExtensionProperties[i].extensionName);
			}
#endif
		}

		auto requiredDeviceExtensions = VulkanPlatform::GetRequiredVulkanDeviceExtensions();

		for (const auto& requiredExtension : requiredDeviceExtensions)
		{
			deviceExtensionNames.Add(requiredExtension);
		}

		deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.GetSize());
		deviceCI.ppEnabledExtensionNames = !deviceExtensionNames.IsEmpty() ? deviceExtensionNames.GetData() : nullptr;

		VkPhysicalDeviceFeatures deviceFeaturesToUse{};
		deviceFeaturesToUse.samplerAnisotropy = VK_TRUE;
#if PLATFORM_DESKTOP
		deviceFeaturesToUse.textureCompressionBC = VK_TRUE;
#endif

		// Descriptor Indexing features
		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
		descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

		// Enable non-uniform indexing
		descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
		descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
		descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

#if PLATFORM_DESKTOP
		deviceCI.pNext = &descriptorIndexingFeatures;
#endif

		deviceCI.pEnabledFeatures = &deviceFeaturesToUse;

		if (vkCreateDevice(gpu, &deviceCI, VULKAN_CPU_ALLOCATOR, &device) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create vulkan device.");
			return;
		}

		// Fetch all queues from the GPU
		FetchQueues(gpu);

		// Command Pools for each family
		for (int familyIdx = 0; familyIdx < queueFamilyProperties.GetSize(); familyIdx++)
		{
			VkCommandPoolCreateInfo commandPoolCI = {};
			commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolCI.queueFamilyIndex = familyIdx;

			VkCommandPool commandPool = nullptr;
			if (vkCreateCommandPool(device, &commandPoolCI, VULKAN_CPU_ALLOCATOR, &commandPool) != VK_SUCCESS)
			{
				CE_LOG(Error, All, "Failed to create Vulkan Command Pool!");
				return;
			}

			for (int i = 0; i < queues.GetSize(); i++)
			{
				if (queues[i]->GetFamilyIndex() == familyIdx)
				{
					queues[i]->commandPool = commandPool;
				}
			}

			queueFamilyToCmdPool[familyIdx] = commandPool;
		}

		// Initialize device limits
		deviceLimits = new Vulkan::DeviceLimits(this);
	}

	void VulkanDevice::FetchQueues(VkPhysicalDevice gpu)
	{
		for (int familyIdx = 0; familyIdx < queueFamilyProperties.GetSize(); familyIdx++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[familyIdx];
			RHI::HardwareQueueClassMask queueMask{};
			if (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				queueMask |= RHI::HardwareQueueClassMask::Compute;
			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				queueMask |= RHI::HardwareQueueClassMask::Graphics;
			if (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
				queueMask |= RHI::HardwareQueueClassMask::Transfer;

			VkBool32 presentationSupported = VK_FALSE;
			if (testSurface != nullptr)
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, familyIdx, testSurface, &presentationSupported);

			bool graphicsQueueAllocated = false;

			for (int i = 0; i < queueFamilyProperty.queueCount; i++)
			{
				bool isLast = (i == (queueFamilyProperty.queueCount - 1));

				VkQueue vkQueue = nullptr;
				vkGetDeviceQueue(device, familyIdx, i, &vkQueue);

				CommandQueue* queue = new CommandQueue(this, familyIdx, i, queueMask, vkQueue, presentationSupported > 0);
				queues.Add(queue);
				queuesByFamily[familyIdx].Add(queue);
                if (presentationSupported)
                {
                    presentQueues.Add(queue);
                }

				if (primaryGraphicsQueue == nullptr && 
					EnumHasAllFlags(queueMask, RHI::HardwareQueueClassMask::Compute | RHI::HardwareQueueClassMask::Graphics) &&
					isLast)
				{
					graphicsQueueAllocated = true;
					primaryGraphicsQueue = queue;
					if (presentationSupported)
						presentQueue = queue;
				}
				else if (presentQueue == nullptr && presentationSupported)
				{
					presentQueue = queue;
				}
			}

			int queueCount = queueFamilyProperty.queueCount;

			// Get the last queue of this family as primary transfer queue
			if (EnumHasFlag(queueMask, RHI::HardwareQueueClassMask::Transfer) && primaryTransferQueue == nullptr &&
                (!graphicsQueueAllocated || queueCount >= 2) && queueCount >= 2)
			{
				primaryTransferQueue = queuesByFamily[familyIdx][queueCount - 2];
			}
		}

		if (primaryTransferQueue == nullptr)
		{
			primaryTransferQueue = primaryGraphicsQueue;
		}
	}

	bool VulkanDevice::QueryGpu(u32 gpuIndex)
	{
		u32 physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices{ physicalDeviceCount };
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		VkPhysicalDevice gpu = physicalDevices[gpuIndex];
		auto queueFamilies = GetQueueFamilies(gpu);

		if (!queueFamilies.IsValidGraphicsFamily() || !queueFamilies.IsValidPresentFamily() || 
			!queueFamilies.IsValidComputeFamily() || !queueFamilies.IsValidTransferFamily())
		{
			return false;
		}

		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

		if (extensionCount == 0)
			return false;

		Array<VkExtensionProperties> supportedExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, supportedExtensions.GetData());

		Array<const char*> requiredVulkanDeviceExtensions = VulkanPlatform::GetRequiredVulkanDeviceExtensions();

		// Check for required extensions in the Physical Device's supported extensions.
		for (auto& requiredExtension : requiredVulkanDeviceExtensions)
		{
			bool hasExtension = false;
			for (const auto& SupportedExtension : supportedExtensions)
			{
				if (strcmp(SupportedExtension.extensionName, requiredExtension) == 0)
				{
					hasExtension = true;
					break;
				}
			}
			if (!hasExtension)
			{
				return false;
			}
		}
		
		auto surfaceSupportInfo = FetchSurfaceSupportInfo(gpu);
		bool surfaceSupportsSwapChain = !surfaceSupportInfo.presentationModes.IsEmpty() && !surfaceSupportInfo.surfaceFormats.IsEmpty();
		if (!surfaceSupportsSwapChain)
			return false;

		return true;
	}

	VulkanQueueFamilies VulkanDevice::GetQueueFamilies(VkPhysicalDevice gpu)
	{
		VulkanQueueFamilies queueFamilies{};

		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
		Array<VkQueueFamilyProperties> queueFamilyProps{ queueFamilyCount };
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProps.GetData());

		for (int i = 0; i < queueFamilyProps.GetSize(); ++i)
		{
			const auto& queueFamilyProperty = queueFamilyProps[i];

			// Always prefer a single Graphics Queue Family that supports all 3: Graphics, Presentation, Compute
			if (testSurface != nullptr && queueFamilyProperty.queueCount > 0 &&
				(queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				VkBool32 presentationSupported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, testSurface, &presentationSupported);
				if (presentationSupported)
				{
					queueFamilies.graphicsFamily = queueFamilies.transferFamily = queueFamilies.presentFamily = i;
				}
				else if (!queueFamilies.IsValidGraphicsFamily())
				{
					queueFamilies.graphicsFamily = queueFamilies.transferFamily = i;
				}
				if (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					queueFamilies.computeFamily = i;
				}
			}

			// If we couldn't find a Graphics Queue with our preferred features, then pick any one
			if (!queueFamilies.IsValidGraphicsFamily() &&
				queueFamilyProperty.queueCount > 0 &&
				queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueFamilies.graphicsFamily = i;
			}

			// Pick any presentation queue if we couldn't find our preferred one
			if (testSurface != nullptr && !queueFamilies.IsValidPresentFamily() && queueFamilyProperty.queueCount > 0)
			{
				VkBool32 PresentationSupported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, testSurface, &PresentationSupported);
				if (PresentationSupported)
				{
					queueFamilies.presentFamily = i;
				}
			}

			// Pick any transfer family if we couldn't find our preferred one
			if (!queueFamilies.IsValidTransferFamily() &&
				queueFamilyProperty.queueFlags > 0 &&
				queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				queueFamilies.transferFamily = i;
			}

			// Pick any Compute family if we couldn't find our preferred one
			if (!queueFamilies.IsValidComputeFamily() &&
				queueFamilyProperty.queueCount > 0 &&
				queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				queueFamilies.computeFamily = i;
			}
		}

		return queueFamilies;
	}

	VkDeviceSize VulkanDevice::GetPhysicalDeviceLocalMemory(VkPhysicalDevice gpu)
	{
		// Determine the available pDevice local memory.
		VkPhysicalDeviceMemoryProperties memoryProps;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProps);

		auto heapsPtr = memoryProps.memoryHeaps;
		auto heaps = std::vector<VkMemoryHeap>(heapsPtr, heapsPtr + memoryProps.memoryHeapCount);
		for (const auto& heap : heaps)
		{
			if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				return heap.size;
			}
		}
		return 0;
	}

	SurfaceSupportInfo VulkanDevice::FetchSurfaceSupportInfo(VkPhysicalDevice gpu)
	{
		SurfaceSupportInfo surfaceSupport = {};

		if (testSurface == nullptr)
			return surfaceSupport;

		// -- CAPABILITIES --
		// Get the surface capabilities for the given physical pDevice
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, testSurface, &surfaceSupport.surfaceCapabilities);

		// -- FORMATS --
		// Retrieve the list of supported formats
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, testSurface, &formatCount, nullptr);
		if (formatCount > 0)
		{
			surfaceSupport.surfaceFormats.Resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, testSurface, &formatCount, surfaceSupport.surfaceFormats.GetData());
		}

		// -- PRESENTATION MODES --
		// Retrieve the list of supported presentation modes
		uint32_t presentationCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, testSurface, &presentationCount, nullptr);
		if (presentationCount > 0)
		{
			surfaceSupport.presentationModes.Resize(presentationCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, testSurface, &presentationCount, surfaceSupport.presentationModes.GetData());
		}

		return surfaceSupport;
	}

	void VulkanDevice::WaitUntilIdle()
	{
		vkDeviceWaitIdle(device);
	}

	VkFormat VulkanDevice::FindSupportedFormat(const Array<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (const auto& format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		return VK_FORMAT_UNDEFINED;
	}

	VkSurfaceFormatKHR VulkanDevice::FindAutoColorFormat()
	{
		bool formatSelected = false;
		const auto& surfaceFormats = surfaceSupport.surfaceFormats;
		VkSurfaceFormatKHR selectedSurfaceFormat{};

		if (surfaceFormats.GetSize() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			selectedSurfaceFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			formatSelected = true;
		}
		else
		{
			// Find our preferred format first
			for (const auto& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
				{
					selectedSurfaceFormat = { surfaceFormat.format, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
					formatSelected = true;
					break;
				}
			}

			if (!formatSelected)
			{
				// RGBA-8 is first preferred and BGRA-8 is second preferred format
				for (const auto& surfaceFormat : surfaceFormats)
				{
					if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM || surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
					{
						selectedSurfaceFormat = { surfaceFormat.format, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
						formatSelected = true;
						break;
					}
				}
			}
		}

		return surfaceFormats[0];
	}

	bool VulkanDevice::CheckSurfaceFormatSupport(VkFormat format)
	{
		const auto& surfaceFormats = surfaceSupport.surfaceFormats;

		for (const auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == format)
			{
				return true;
			}
		}

		return false;
	}

	s32 VulkanDevice::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

		for (u32 i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		return -1;
	}

	VkImageView VulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageViewType imageViewType, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image;
		imageViewCI.viewType = imageViewType;
		imageViewCI.format = format;

		// Allows remapping of RGBA components to other channel values
		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Subresources allow the view to view only a part of an image
		imageViewCI.subresourceRange.aspectMask = aspectFlags;  // Which aspect of the image to view (e.g. color bit for viewing color)
		imageViewCI.subresourceRange.baseMipLevel = 0;          // Start mipmap level to view from
		imageViewCI.subresourceRange.levelCount = 1;            // No. of mipmap levels to view
		imageViewCI.subresourceRange.baseArrayLayer = 0;        // Start array layer to view from
		imageViewCI.subresourceRange.layerCount = 1;            // No. of array layers to view

		// CreateGraphicsPipeline image view and return it
		VkImageView imageView;
		if (vkCreateImageView(GetHandle(), &imageViewCI, VULKAN_CPU_ALLOCATOR, &imageView) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create Vulkan Image View!");
			return nullptr;
		}
		return imageView;
	}

	int VulkanDevice::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, 
		const VkImageSubresourceRange& subresource,
		VkImageAspectFlags aspectMask)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange = subresource;
		//barrier.subresourceRange.aspectMask = aspectMask;
		//barrier.subresourceRange.levelCount = 1;
		//barrier.subresourceRange.baseArrayLayer = 0;
		//barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage = 0;
		VkPipelineStageFlags destinationStage = 0;
		
		switch (oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		default:
			return -1;
		}

		destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
				barrier.srcAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			destinationStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			barrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		default:
			return -1;
		}

		VkCommandBuffer commandBuffer = nullptr;
		VkCommandPool commandPool = commandAllocator->Allocate(1, &commandBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, primaryGraphicsQueue->GetFamilyIndex());

		defer(&)
		{
			commandAllocator->Free(commandPool, 1, &commandBuffer);
		};

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(commandBuffer);

		VkFenceCreateInfo fenceCI{};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence fence = nullptr;
		vkCreateFence(device, &fenceCI, VULKAN_CPU_ALLOCATOR, &fence);

		defer(&)
		{
			vkDestroyFence(device, fence, VULKAN_CPU_ALLOCATOR);
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		primaryGraphicsQueue->Submit(1, &submitInfo, fence);
		vkWaitForFences(device, 1, &fence, VK_TRUE, NumericLimits<u64>::Max());

		return primaryGraphicsQueue->GetFamilyIndex();
	}

	VkCommandBuffer VulkanDevice::BeginSingleUseCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = gfxCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanDevice::EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
	{
        vkEndCommandBuffer(commandBuffer);
	}

    int VulkanDevice::SubmitAndWaitSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

		VkFenceCreateInfo fenceCI{};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        //vkQueueSubmit(primaryGraphicsQueue->GetHandle(), 1, &submitInfo, VK_NULL_HANDLE);
		primaryGraphicsQueue->Submit(1, &submitInfo, VK_NULL_HANDLE);

		{
			LockGuard guard{ primaryGraphicsQueue->GetMutex() };
        	vkQueueWaitIdle(primaryGraphicsQueue->GetHandle());
		}

        vkFreeCommandBuffers(device, gfxCommandPool, 1, &commandBuffer);

		return primaryGraphicsQueue->familyIndex;
    }

	VkCommandPool VulkanDevice::AllocateCommandBuffers(u32 count, VkCommandBuffer* outBuffers, RHI::CommandListType type, u32 queueFamilyIndex)
	{
		return commandAllocator->Allocate(count, outBuffers, 
			type == RHI::CommandListType::Indirect ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			queueFamilyIndex);
	}

	Array<RHI::CommandQueue*> VulkanDevice::GetHardwareQueues(RHI::HardwareQueueClassMask queueMask)
	{
		Array<RHI::CommandQueue*> result{};

		for (int i = 0; i < queues.GetSize(); i++)
		{
			if ((queues[i]->GetQueueMask() & queueMask) == queueMask)
				result.Add(queues[i]);
		}

		return result;
	}

	Array<RHI::CommandQueue*> VulkanDevice::AllocateHardwareQueues(const HashMap<RHI::HardwareQueueClass, int>& queueCountByClass)
	{
		Array<RHI::CommandQueue*> queues{};
		int queueFamilyCount = queueFamilyProperties.GetSize();
		HashMap<RHI::HardwareQueueClass, int> queueCounts = queueCountByClass;

		int& graphicsQueueCount = queueCounts[RHI::HardwareQueueClass::Graphics];
		int& transferQueueCount = queueCounts[RHI::HardwareQueueClass::Transfer];
		int& computeQueueCount = queueCounts[RHI::HardwareQueueClass::Compute];

		for (int familyIdx = 0; familyIdx < queueFamilyCount; familyIdx++)
		{
			VkQueueFamilyProperties familyProps = queueFamilyProperties[familyIdx];
			
			if (graphicsQueueCount > 0 && (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				if (graphicsQueueCount < familyProps.queueCount)
					graphicsQueueCount = 0;
				else
					graphicsQueueCount -= familyProps.queueCount;
			}

		}

		return queues;
	}

} // namespace CE
