
#include "CoreMinimal.h"

#include "VulkanRHI.h"
#include "VulkanRHIPrivate.h"
#include "PAL/Common/VulkanPlatform.h"
#undef max

#include <vulkan/vulkan.h>

CE_IMPLEMENT_MODULE(VulkanRHI, CE::Vulkan::VulkanRHIModule)

namespace CE::Vulkan
{
	static VulkanRHI* gVulkanRHI = nullptr;

	void VulkanRHIModule::StartupModule()
	{
        //RHI::gDynamicRHI = new VulkanRHI();
	}

	void VulkanRHIModule::ShutdownModule()
	{
        delete RHI::gDynamicRHI;
	}

	void VulkanRHIModule::RegisterTypes()
	{

	}

    static SIZE_T gAllocatedMemory = 0;
    static SharedMutex gAllocMutex{};
    static HashMap<void*, SIZE_T> gAlignmentMap;
    
    static void* VkAllocateMemoryFunc(void* pUserData, size_t size, size_t  alignment, VkSystemAllocationScope allocationScope)
    {
        LockGuard lock{ gAllocMutex };
        gAllocatedMemory += Memory::GetAlignedSize(size, alignment);
        void* block = Memory::AlignedAlloc(size, alignment);
        gAlignmentMap[block] = alignment;
        return block;
    }

    static void* VkReallocateMemoryFunc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        LockGuard lock{ gAllocMutex };
        gAllocatedMemory += Memory::GetAlignedSize(size, alignment);
        void* block = Memory::AlignedRealloc(pOriginal, size, alignment);
        gAlignmentMap[block] = alignment;
        return block;
    }

    static void VkFreeMemoryFunc(void* pUserData, void* pMemory)
    {
        if (pMemory != nullptr)
	    {
		    if (gAlignmentMap[pMemory] > 0)
		    {
                LockGuard lock{ gAllocMutex };
                gAllocatedMemory -= Memory::AlignedBlockSize(pMemory, gAlignmentMap[pMemory], 0);
                gAlignmentMap.Remove(pMemory);
		    }
            Memory::AlignedFree(pMemory);
	    }
    }

    VkAllocationCallbacks gVulkanAllocators = {
	    nullptr,                 /* pUserData;             */
        VkAllocateMemoryFunc,            /* pfnAllocation;         */
        VkReallocateMemoryFunc,          /* pfnReallocation;       */
        VkFreeMemoryFunc,             /* pfnFree;               */
	    nullptr,                 /* pfnInternalAllocation; */
        nullptr	    /* pfnInternalFree;       */
    };

    VKAPI_ATTR VkBool32 VulkanValidationCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
			if (gVulkanRHI != nullptr)
				gVulkanRHI->BroadCastValidationMessage(RHI::ValidationMessageType::Error, pCallbackData->pMessage);
            CE_LOG(Error, All, "Vulkan Error: {}", pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
			if (gVulkanRHI != nullptr)
				gVulkanRHI->BroadCastValidationMessage(RHI::ValidationMessageType::Info, pCallbackData->pMessage);
            CE_LOG(Info, All, "Vulkan Info: {}", pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
			if (gVulkanRHI != nullptr)
				gVulkanRHI->BroadCastValidationMessage(RHI::ValidationMessageType::Verbose, pCallbackData->pMessage);
            CE_LOG(Info, All, "Vulkan Verbose: {}", pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
			if (gVulkanRHI != nullptr)
				gVulkanRHI->BroadCastValidationMessage(RHI::ValidationMessageType::Warning, pCallbackData->pMessage);
            CE_LOG(Warn, All, "Vulkan Warning: {}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        if (VulkanPlatform::IsVerboseValidationEnabled())
            createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = VulkanValidationCallback;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

	void VulkanRHI::Initialize()
	{
		gVulkanRHI = this;

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_1;
        appInfo.pEngineName = CE_ENGINE_NAME_STRING;
        appInfo.engineVersion = VK_MAKE_VERSION(CE_VERSION_MAJOR, CE_VERSION_MINOR, CE_VERSION_PATCH);
        appInfo.pApplicationName = gProjectName.GetCString();
        appInfo.applicationVersion = appInfo.engineVersion;
        
        Array<const char*> validationLayers = VulkanPlatform::GetValidationLayers();
        bool enableValidation = VulkanPlatform::IsValidationEnabled();
        Array<const char*> instanceExtensions = VulkanPlatform::GetRequiredInstanceExtensions();
        
        u32 extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        Array<VkExtensionProperties> extensionProps{}; extensionProps.Resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProps.GetData());

        for (int i = 0; i < extensionCount; i++)
        {
            auto tryAddExtension = [&](const char* extName)
                {
                    if (strcmp(extensionProps[i].extensionName, extName) == 0)
                    {
                        instanceExtensions.Add(extensionProps[i].extensionName);
                        return true;
                    }
                    return false;
                };

            tryAddExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        VkInstanceCreateInfo instanceCI{};
        instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCI.pApplicationInfo = &appInfo;
        instanceCI.flags = VulkanPlatform::GetVulkanInstanceFlags();

        VkDebugUtilsMessengerCreateInfoEXT debugCI{};
        debugCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCI.pNext = nullptr;

        instanceCI.enabledExtensionCount = instanceExtensions.GetSize();
        instanceCI.ppEnabledExtensionNames = instanceExtensions.GetData();

        this->instanceExtensions = instanceExtensions;

        if (enableValidation)
        {
            instanceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetSize());
            instanceCI.ppEnabledLayerNames = validationLayers.GetData();

            PopulateDebugMessengerCreateInfo(debugCI);
            instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCI;
        }
        else
        {
            instanceCI.enabledLayerCount = 0;
            instanceCI.ppEnabledLayerNames = nullptr;
            instanceCI.pNext = nullptr;
        }
        
        u32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        Array<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetData());
        
        VkResult result = vkCreateInstance(&instanceCI, VULKAN_CPU_ALLOCATOR, &vkInstance);
        if (result != VK_SUCCESS)
        {
            CE_LOG(Critical, All, "Failed to create vulkan instance. Result = {}" , (int)result);
            return;
        }

        if (enableValidation && CreateDebugUtilsMessengerEXT(vkInstance, &debugCI, VULKAN_CPU_ALLOCATOR, &vkMessenger) != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create Vulkan debug messenger!");
            return;
        }
	}

    void VulkanRHI::PostInitialize()
    {
        device = new VulkanDevice(vkInstance, this);
        device->Initialize();
    }

	void VulkanRHI::PreShutdown()
	{
        if (device != nullptr)
        {
            device->PreShutdown();
        }
	}

	void VulkanRHI::Shutdown()
	{
        if (device != nullptr)
        {
            device->Shutdown();
            delete device;
            device = nullptr;
        }

        if (VulkanPlatform::IsValidationEnabled())
        {
            DestroyDebugUtilsMessengerEXT(vkInstance, vkMessenger, VULKAN_CPU_ALLOCATOR);
        }

        vkDestroyInstance(vkInstance, VULKAN_CPU_ALLOCATOR);
        vkInstance = nullptr;

		gVulkanRHI = nullptr;
	}

    void* VulkanRHI::GetNativeHandle()
    {
        return vkInstance;
    }

    RHI::GraphicsBackend VulkanRHI::GetGraphicsBackend()
	{
		return RHI::GraphicsBackend::Vulkan;
	}

    // - Render Target -

	RHI::Scope* VulkanRHI::CreateScope(const RHI::ScopeDescriptor& desc)
	{
		return new Vulkan::Scope(device, desc);
	}

	RHI::FrameGraphCompiler* VulkanRHI::CreateFrameGraphCompiler()
	{
		return new Vulkan::FrameGraphCompiler(device);
	}

	RHI::FrameGraphExecuter* VulkanRHI::CreateFrameGraphExecuter()
	{
		return new Vulkan::FrameGraphExecuter(device);
	}

	void VulkanRHI::BroadCastValidationMessage(RHI::ValidationMessageType type, const char* message)
	{
		for (auto handler : validationCallbackHandlers[(int)type])
		{
			if (handler)
				handler(type, message);
		}
	}

	bool VulkanRHI::IsOffscreenOnly()
	{
		return device->IsOffscreenOnly();
	}

	Vec2i VulkanRHI::GetScreenSizeForWindow(void* platformWindowHandle)
	{
		return VulkanPlatform::GetScreenSizeForWindow(platformWindowHandle);
	}

    RHI::Fence* VulkanRHI::CreateFence(bool initiallySignalled)
    {
        return new Vulkan::Fence(device, initiallySignalled);
    }

    void VulkanRHI::DestroyFence(RHI::Fence* fence)
    {
        delete fence;
    }

	Array<RHI::Format> VulkanRHI::GetAvailableDepthStencilFormats()
	{
		return device->GetAvailableDepthStencilFormats();
	}

    Array<RHI::Format> VulkanRHI::GetAvailableDepthOnlyFormats()
    {
        return device->GetAvailableDepthOnlyFormats();
    }

    // - Command List -

    RHI::CommandList* VulkanRHI::AllocateCommandList(RHI::CommandQueue* associatedQueue, RHI::CommandListType commandListType)
    {
        if (associatedQueue == nullptr)
            return nullptr;

        Vulkan::CommandQueue* vulkanQueue = (Vulkan::CommandQueue*)associatedQueue;
        VkCommandBuffer outCmdBuffer = nullptr;
        VkCommandPool cmdPool = device->AllocateCommandBuffers(1, &outCmdBuffer, commandListType, vulkanQueue->GetFamilyIndex());
        return new Vulkan::CommandList(device, outCmdBuffer, commandListType, vulkanQueue->GetFamilyIndex(), cmdPool);
    }

    Array<RHI::CommandList*> VulkanRHI::AllocateCommandLists(u32 count, RHI::CommandQueue* associatedQueue, RHI::CommandListType commandListType)
    {
        if (associatedQueue == nullptr)
            return {};

        Array<RHI::CommandList*> result{};
        List<VkCommandBuffer> outCmdBuffers{};
        result.Resize(count);
        outCmdBuffers.Resize(count);

        Vulkan::CommandQueue* vulkanQueue = (Vulkan::CommandQueue*)associatedQueue;
        VkCommandPool cmdPool = device->AllocateCommandBuffers(count, outCmdBuffers.GetData(), commandListType, vulkanQueue->GetFamilyIndex());

        for (int i = 0; i < count; i++)
        {
            result[i] = new Vulkan::CommandList(device, outCmdBuffers[i], commandListType, vulkanQueue->GetFamilyIndex(), cmdPool);
        }

        return result;
    }

    void VulkanRHI::FreeCommandLists(u32 count, RHI::CommandList** commandLists)
    {
        for (int i = 0; i < count; i++)
        {
            delete commandLists[i];
        }
    }

    // - Resources -

    RHI::DeviceLimits* VulkanRHI::GetDeviceLimits()
    {
        return device->deviceLimits;
    }

    RHI::RenderTarget* VulkanRHI::CreateRenderTarget(const RHI::RenderTargetLayout& rtLayout)
    {
        return new Vulkan::RenderTarget(device, rtLayout);
    }

    void VulkanRHI::DestroyRenderTarget(RHI::RenderTarget* renderTarget)
    {
        delete renderTarget;
    }

    RHI::RenderTargetBuffer* VulkanRHI::CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::Texture*>& imageAttachments, u32 imageIndex)
    {
        Vulkan::RenderTarget* vkRenderTarget = (Vulkan::RenderTarget*)renderTarget;
        if (vkRenderTarget == nullptr)
            return nullptr;
        Array<Vulkan::Texture*> textures{};
        textures.Resize(imageAttachments.GetSize());
        for (int i = 0; i < imageAttachments.GetSize(); i++)
        {
            textures[i] = (Vulkan::Texture*)imageAttachments[i];
        }
        return new FrameBuffer(device, textures, vkRenderTarget->GetRenderPass(), 0);
    }

    RHI::RenderTargetBuffer* VulkanRHI::CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, 
        const Array<RHI::TextureView*>& imageAttachments, u32 imageIndex)
    {
        Vulkan::RenderTarget* vkRenderTarget = (Vulkan::RenderTarget*)renderTarget;
        if (vkRenderTarget == nullptr)
            return nullptr;
        Array<Vulkan::TextureView*> textureViews{};
        textureViews.Resize(imageAttachments.GetSize());
        for (int i = 0; i < imageAttachments.GetSize(); i++)
        {
            textureViews[i] = (Vulkan::TextureView*)imageAttachments[i];
        }
        return new FrameBuffer(device, textureViews, vkRenderTarget->GetRenderPass(), 0);
    }

    void VulkanRHI::DestroyRenderTargetBuffer(RHI::RenderTargetBuffer* renderTargetBuffer)
    {
        delete renderTargetBuffer;
    }

    RHI::SwapChain* VulkanRHI::CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc)
	{
		return new Vulkan::SwapChain(this, device, window, desc);
	}

	void VulkanRHI::DestroySwapChain(RHI::SwapChain* swapChain)
	{
		delete swapChain;
	}

	RHI::MemoryHeap* VulkanRHI::AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc)
	{
		return new MemoryHeap(device, desc);
	}

	void VulkanRHI::FreeMemoryHeap(RHI::MemoryHeap* memoryHeap)
	{
		delete memoryHeap;
	}

	RHI::Buffer* VulkanRHI::CreateBuffer(const RHI::BufferDescriptor& bufferDesc)
    {
        return new Buffer(device, bufferDesc);
    }

	RHI::Buffer* VulkanRHI::CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc)
	{
		return new Buffer(device, bufferDesc, memoryDesc);
	}

    void VulkanRHI::DestroyBuffer(RHI::Buffer* buffer)
    {
        delete buffer;
    }

    RHI::TextureView* VulkanRHI::CreateTextureView(const RHI::TextureViewDescriptor& desc)
    {
        return new Vulkan::TextureView(device, desc);
    }

    void VulkanRHI::DestroyTextureView(RHI::TextureView* textureView)
    {
        delete textureView;
    }

    RHI::Texture* VulkanRHI::CreateTexture(const RHI::TextureDescriptor& textureDesc)
    {
        return new Texture(device, textureDesc);
    }

	RHI::Texture* VulkanRHI::CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc)
	{
		return new Texture(device, textureDesc, memoryDesc);
	}

    void VulkanRHI::DestroyTexture(RHI::Texture* texture)
    {
        delete texture;
    }

    RHI::Sampler* VulkanRHI::CreateSampler(const RHI::SamplerDescriptor& samplerDesc)
    {
        return new Sampler(device, samplerDesc);
    }

    void VulkanRHI::DestroySampler(RHI::Sampler* sampler)
    {
        delete sampler;
    }

	RHI::ShaderModule* VulkanRHI::CreateShaderModule(const RHI::ShaderModuleDescriptor& desc)
	{
		return new Vulkan::ShaderModule(device, desc);
	}

	void VulkanRHI::DestroyShaderModule(RHI::ShaderModule* shaderModule)
	{
		delete shaderModule;
	}

	RHI::ShaderResourceGroup* VulkanRHI::CreateShaderResourceGroup(const RHI::ShaderResourceGroupLayout& srgLayout)
	{
		return new Vulkan::ShaderResourceGroup(device, srgLayout);
	}

	void VulkanRHI::DestroyShaderResourceGroup(RHI::ShaderResourceGroup* shaderResourceGroup)
	{
		delete shaderResourceGroup;
	}

    RHI::PipelineState* VulkanRHI::CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc)
    {
        return new Vulkan::PipelineState(device, desc);
    }

    RHI::PipelineState* VulkanRHI::CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc)
    {
        return new Vulkan::PipelineState(device, desc);
    }

    void VulkanRHI::DestroyPipeline(const RHI::PipelineState* pipeline)
    {
        delete pipeline;
    }

	Array<RHI::CommandQueue*> VulkanRHI::GetHardwareQueues(RHI::HardwareQueueClassMask queueMask)
	{
		return device->GetHardwareQueues(queueMask);
	}

    RHI::CommandQueue* VulkanRHI::GetPrimaryGraphicsQueue()
    {
        return device->primaryGraphicsQueue;
    }

    RHI::CommandQueue* VulkanRHI::GetPrimaryTransferQueue()
    {
        return device->primaryTransferQueue;
    }

    u64 VulkanRHI::GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member)
    {
        u64 alignment = 0;

        switch (member.dataType)
        {
        case RHI::ShaderStructMemberType::UInt:
        case RHI::ShaderStructMemberType::Int:
        case RHI::ShaderStructMemberType::Float:
            return sizeof(u32); // 4 byte
        case RHI::ShaderStructMemberType::Float2:
            return sizeof(f32) * 2; // 8 bytes
        case RHI::ShaderStructMemberType::Float3:
        case RHI::ShaderStructMemberType::Float4:
        case RHI::ShaderStructMemberType::Float4x4:
            return sizeof(f32) * 4; // 16 bytes
        case RHI::ShaderStructMemberType::Struct:
            alignment = 0;
            for (const auto& nestedMember : member.nestedMembers)
            {
                alignment = Math::Max(alignment, GetShaderStructMemberAlignment(nestedMember));
            }
            return alignment;
        }

        return alignment;
    }

    u64 VulkanRHI::GetShaderStructMemberSize(const RHI::ShaderStructMember& member)
    {
        switch (member.dataType)
        {
        case RHI::ShaderStructMemberType::Float:
        case RHI::ShaderStructMemberType::UInt:
        case RHI::ShaderStructMemberType::Int:
            return sizeof(u32) * member.arrayCount;
        case RHI::ShaderStructMemberType::Float2:
            return sizeof(Vec2) * member.arrayCount;
        case RHI::ShaderStructMemberType::Float3:
        case RHI::ShaderStructMemberType::Float4:
            return sizeof(Vec4) * member.arrayCount;
        case RHI::ShaderStructMemberType::Float4x4:
            return sizeof(Matrix4x4);
        case RHI::ShaderStructMemberType::Struct:
        {
            u64 structAlignment = GetShaderStructMemberAlignment(member);
            u64 offset = 0;
            for (const auto& nestedMember : member.nestedMembers)
            {
                u64 alignment = GetShaderStructMemberAlignment(nestedMember);
                if (offset > 0)
                    offset = Memory::GetAlignedSize(offset, alignment);
                offset += GetShaderStructMemberSize(nestedMember);
            }
            offset = Memory::GetAlignedSize(offset, structAlignment);
            return offset;
        }
        break;
        }

        return 0;
    }

    void VulkanRHI::GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets)
    {
        outOffsets.Clear();

        u64 offset = 0;
        
        for (const auto& member : members)
        {
            u64 alignment = GetShaderStructMemberAlignment(member);
            if (offset > 0)
                offset = Memory::GetAlignedSize(offset, alignment);
            outOffsets.Add(offset);
            offset += GetShaderStructMemberSize(member);
        }
    }

    RHI::ResourceMemoryRequirements VulkanRHI::GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList)
    {
        if (count == 0)
            return {};
        
        RHI::ResourceMemoryRequirements result{};
        u64 offset = 0;
        result.flags = requirementsList[0].flags;
        result.size = requirementsList[0].size;
        result.offsetAlignment = requirementsList[0].offsetAlignment;
        if (outOffsetsList)
            outOffsetsList[0] = offset;
        offset += requirementsList[0].size;

        for (int i = 1; i < count; i++)
        {
            result.offsetAlignment = 0;
            result.flags &= requirementsList[i].flags;

            if (offset > 0)
                offset = Memory::GetAlignedSize(offset, requirementsList[i].offsetAlignment);
            if (outOffsetsList)
                outOffsetsList[i] = offset;

            offset += requirementsList[i].size;
        }

        result.size = offset;

        return result;
    }

} // namespace CE
