#pragma once

#include "Core.h"
#include "CoreRHI.h"

struct VkInstance_T;
typedef VkInstance_T* VkInstance;

struct VkDebugUtilsMessengerEXT_T;
typedef VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

namespace CE::Vulkan
{
    class Device;
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

        void WaitToShutdown() override;
        
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

		virtual Array<RHI::CommandQueue*> GetHardwareQueues(RHI::HardwareQueueClass queueClass) override;

        virtual RHI::CommandQueue* GetPrimaryGraphicsQueue() override;
        virtual RHI::CommandQueue* GetPrimaryTransferQueue() override;

		// TODO: move this function to CoreApplication instead of VulkanRHI
		Vec2i GetScreenSizeForWindow(void* platformWindowHandle) override;

        // - Command List -

        virtual RHI::Fence* CreateFence(uint64_t initialValue) override;

        virtual void DestroyFence(RHI::Fence* fence) override;

        virtual RHI::CommandList* AllocateCommandList(RHI::CommandQueue* associatedQueue,
                                                      RHI::CommandListType commandListType = RHI::CommandListType::Direct) override;

        virtual Array<RHI::CommandList*> AllocateCommandLists(u32 count, RHI::CommandQueue* associatedQueue,
                                                              RHI::CommandListType commandListType = RHI::CommandListType::Direct) override;

        virtual void FreeCommandLists(u32 count, RHI::CommandList** commandLists) override;

        // - Resources -

        virtual RHI::DeviceLimits* GetDeviceLimits() override;

		virtual RHI::SwapChain* CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc) override;
		virtual void DestroySwapChain(RHI::SwapChain* swapChain) override;

		virtual RHI::MemoryHeap* AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc) override;
		virtual void FreeMemoryHeap(RHI::MemoryHeap* memoryHeap) override;

        RHI::AliasedHeap* AllocateAliasedHeap(const RHI::AliasedHeapDescriptor& desc) override;
        void FreeAliasedHeap(RHI::AliasedHeap* heap) override;

		virtual void GetBufferMemoryRequirements(const RHI::BufferDescriptor& bufferDesc, RHI::ResourceMemoryRequirements& outRequirements) override;
        
        virtual void GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements) override;

        virtual RHI::ResourceMemoryRequirements GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList = nullptr) override;

        virtual RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc) override;
		virtual RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) override;
        RHI::Buffer* CreateBufferAliased(const RHI::BufferDescriptor& bufferDesc, Ptr<RHI::AliasedHeap> aliasedHeap, u64 memoryOffset) override;
        virtual void DestroyBuffer(RHI::Buffer* buffer) override;

        virtual RHI::TextureView* CreateTextureView(const RHI::TextureViewDescriptor& desc) override;
        RHI::TextureView* CreateDefaultTextureView(RHI::Texture* texture) override;
        virtual void DestroyTextureView(RHI::TextureView* textureView) override;
        
        virtual RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc) override;
		virtual RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) override;
        virtual void DestroyTexture(RHI::Texture* texture) override;
        
        virtual RHI::Sampler* CreateSampler(const RHI::SamplerDescriptor& samplerDesc) override;
        virtual void DestroySampler(RHI::Sampler* sampler) override;

		virtual RHI::ShaderModule* CreateShaderModule(const RHI::ShaderModuleDescriptor& desc) override;
		virtual void DestroyShaderModule(RHI::ShaderModule* shaderModule) override;
		
		virtual RHI::ShaderResourceGroup* CreateShaderResourceGroup(const RHI::ShaderResourceGroupDescriptor& srgDescriptor) override;
		virtual void DestroyShaderResourceGroup(RHI::ShaderResourceGroup* shaderResourceGroup) override;

        virtual RHI::RayTracingBlas* CreateRayTracingBlas(const RHI::RayTracingBlasDescriptor& desc) override;
        virtual void DestroyRayTracingBlas(RHI::RayTracingBlas* blas) override;

        RHI::RayTracingTlas* CreateRayTracingTlas(const RHI::RayTracingTlasDescriptor& desc) override;
        void DestroyRayTracingTlas(RHI::RayTracingTlas* tlas) override;

		// - Pipeline State -

        virtual RHI::PipelineState* CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc) override;
		virtual RHI::PipelineState* CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc) override;
        virtual void DestroyPipeline(const RHI::PipelineState* pipeline) override;

		// - Utilities -

        virtual u64 GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member) override;
        virtual u64 GetShaderStructMemberSize(const RHI::ShaderStructMember& member) override;
        virtual void GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets) override;

        inline Device* GetDevice() const { return device; }

        RHI::RenderPass* CreateRenderPass(const RHI::RenderPassLayout& rpLayout) override;
        void DestroyRenderPass(RHI::RenderPass* renderPass) override;
        RHI::RenderPassFrameBuffer* CreateRenderPassFrameBuffer(const RHI::RenderPassFrameBufferDescriptor& descriptor) override;
        void DestroyRenderPassFrameBuffer(RHI::RenderPassFrameBuffer* frameBuffer) override;

    protected:

    private:
        VkInstance vkInstance = nullptr;
        VkDebugUtilsMessengerEXT vkMessenger = nullptr;

        Device* device = nullptr;
        Array<const char*> instanceExtensions{};

        HashMap<SIZE_T, RHI::ResourceMemoryRequirements> bufferMemoryRequirementCache{};
        HashMap<SIZE_T, RHI::ResourceMemoryRequirements> imageMemoryRequirementCache{};
    };
    
} // namespace CE
