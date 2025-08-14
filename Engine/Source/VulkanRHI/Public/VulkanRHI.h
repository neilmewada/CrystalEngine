#pragma once

#include "Core.h"
#include "CoreRHI.h"

struct VkInstance_T;
typedef VkInstance_T* VkInstance;

struct VkDebugUtilsMessengerEXT_T;
typedef VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

namespace CE::Vulkan
{
    class VulkanDevice;
    class Viewport;
    class GraphicsCommandList;
    class RenderTarget;

    class VULKANRHI_API VulkanRHIModule : public PluginModule
    {
    public:
        VulkanRHIModule() {}
        virtual ~VulkanRHIModule() {}

        virtual void StartupModule() override;
        virtual void ShutdownModule() override;
        virtual void RegisterTypes() override;
        
    };

	namespace Limits
	{
#if PAL_TRAIT_VULKAN_LIMITED_DESCRIPTOR_SETS
		constexpr u32 MaxDescriptorSetCount = 4;
#else
		constexpr u32 MaxDescriptorSetCount = 8;
#endif
	}

    class VULKANRHI_API VulkanRHI : public RHI::DynamicRHI
    {
    public:
        virtual ~VulkanRHI() = default;

        virtual void Initialize() override;
        virtual void PostInitialize() override;
        virtual void PreShutdown() override;
        virtual void Shutdown() override;
        
        virtual void* GetNativeHandle() override;

        VkInstance GetInstanceHandle() const
        {
            return vkInstance;
        }
        
        virtual RHI::GraphicsBackend GetGraphicsBackend() override;

        // ************************************************
        // - Public API -

		// - FrameGraph API -

		virtual RHI::Scope* CreateScope(const RHI::ScopeDescriptor& desc) override;

		virtual RHI::FrameGraphCompiler* CreateFrameGraphCompiler() override;

		virtual RHI::FrameGraphExecuter* CreateFrameGraphExecuter() override;

		// - Utils -

		virtual Array<RHI::Format> GetAvailableDepthStencilFormats() override;
        virtual Array<RHI::Format> GetAvailableDepthOnlyFormats() override;

		void BroadCastValidationMessage(RHI::ValidationMessageType type, const char* message);

		virtual bool IsOffscreenOnly() override;

		virtual Array<RHI::CommandQueue*> GetHardwareQueues(RHI::HardwareQueueClassMask queueMask) override;

        virtual RHI::CommandQueue* GetPrimaryGraphicsQueue() override;
        virtual RHI::CommandQueue* GetPrimaryTransferQueue() override;

		// TODO: move this function to CoreApplication instead of VulkanRHI
		Vec2i GetScreenSizeForWindow(void* platformWindowHandle) override;

        // - Command List -

        virtual RHI::Fence* CreateFence(bool initiallySignalled = false) override;

        virtual void DestroyFence(RHI::Fence* fence) override;

        virtual RHI::CommandList* AllocateCommandList(RHI::CommandQueue* associatedQueue,
                                                      RHI::CommandListType commandListType = RHI::CommandListType::Direct) override;

        virtual Array<RHI::CommandList*> AllocateCommandLists(u32 count, RHI::CommandQueue* associatedQueue,
                                                              RHI::CommandListType commandListType = RHI::CommandListType::Direct) override;

        virtual void FreeCommandLists(u32 count, RHI::CommandList** commandLists) override;

        // - Resources -

        virtual RHI::DeviceLimits* GetDeviceLimits() override;

        virtual RHI::RenderTarget* CreateRenderTarget(const RHI::RenderTargetLayout& rtLayout) override;
        virtual void DestroyRenderTarget(RHI::RenderTarget* renderTarget) override;

        virtual RHI::RenderTargetBuffer* CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::TextureView*>& imageAttachments, u32 imageIndex = 0) override;
        virtual RHI::RenderTargetBuffer* CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::Texture*>& imageAttachments, u32 imageIndex = 0) override;
        virtual void DestroyRenderTargetBuffer(RHI::RenderTargetBuffer* renderTargetBuffer) override;

		virtual RHI::SwapChain* CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc) override;
		virtual void DestroySwapChain(RHI::SwapChain* swapChain) override;

		virtual RHI::MemoryHeap* AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc) override;
		virtual void FreeMemoryHeap(RHI::MemoryHeap* memoryHeap) override;

		virtual void GetBufferMemoryRequirements(const RHI::BufferDescriptor& bufferDesc, RHI::ResourceMemoryRequirements& outRequirements) override;
        
        virtual void GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements) override;

        virtual RHI::ResourceMemoryRequirements GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList = nullptr) override;

        virtual RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc) override;
		virtual RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) override;
        virtual void DestroyBuffer(RHI::Buffer* buffer) override;

        virtual RHI::TextureView* CreateTextureView(const RHI::TextureViewDescriptor& desc) override;
        virtual void DestroyTextureView(RHI::TextureView* textureView) override;
        
        virtual RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc) override;
		virtual RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) override;
        virtual void DestroyTexture(RHI::Texture* texture) override;
        
        virtual RHI::Sampler* CreateSampler(const RHI::SamplerDescriptor& samplerDesc) override;
        virtual void DestroySampler(RHI::Sampler* sampler) override;

		virtual RHI::ShaderModule* CreateShaderModule(const RHI::ShaderModuleDescriptor& desc) override;
		virtual void DestroyShaderModule(RHI::ShaderModule* shaderModule) override;
		
		virtual RHI::ShaderResourceGroup* CreateShaderResourceGroup(const RHI::ShaderResourceGroupLayout& srgLayout) override;
		virtual void DestroyShaderResourceGroup(RHI::ShaderResourceGroup* shaderResourceGroup) override;

		// - Pipeline State -

        virtual RHI::PipelineState* CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc) override;
		virtual RHI::PipelineState* CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc) override;
        virtual void DestroyPipeline(const RHI::PipelineState* pipeline) override;

		// - Utilities -

        virtual u64 GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member) override;
        virtual u64 GetShaderStructMemberSize(const RHI::ShaderStructMember& member) override;
        virtual void GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets) override;

        inline VulkanDevice* GetDevice() const { return device; }

    protected:

    private:
        VkInstance vkInstance = nullptr;
        VkDebugUtilsMessengerEXT vkMessenger = nullptr;

        VulkanDevice* device = nullptr;
        Array<const char*> instanceExtensions{};
    };
    
} // namespace CE
