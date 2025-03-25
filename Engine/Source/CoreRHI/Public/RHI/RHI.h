#pragma once

#include "CoreMinimal.h"

#include "Resources.h"

namespace CE
{
    class PlatformWindow;
	struct ShaderReflection;
}

namespace CE::RHI
{
    class RenderTarget;
    class Viewport;
    class CommandList;
	class CommandQueue;
    class GraphicsCommandList;
	struct BufferDescriptor;
	struct TextureDescriptor;
	struct ResourceMemoryRequirements;
	class DeviceLimits;

	enum class ValidationMessageType
	{
		Verbose = 0,
		Info,
		Warning,
		Error,
		COUNT
	};

	typedef void (*ValidationCallback)(ValidationMessageType, const char*);

    class CORERHI_API DynamicRHI
    {
    public:
        
        virtual ~DynamicRHI() = default;
        
        // Lifecycle
        virtual void Initialize() = 0;
        virtual void PostInitialize() = 0;
        virtual void PreShutdown() = 0;
        virtual void Shutdown() = 0;
        
        virtual void* GetNativeHandle() = 0;

        virtual GraphicsBackend GetGraphicsBackend() = 0;

        // *******************************************
        // - Public API -

		// - FrameGraph -

		virtual RHI::Scope* CreateScope(const ScopeDescriptor& desc) = 0;

		virtual RHI::FrameGraphCompiler* CreateFrameGraphCompiler() = 0;

		virtual RHI::FrameGraphExecuter* CreateFrameGraphExecuter() = 0;

		// - Utils -
        
		void AddValidationCallbackHandler(ValidationCallback handler, ValidationMessageType logLevel)
		{
			for (int i = (int)logLevel; i < validationCallbackHandlers.GetCapacity(); i++)
			{
				validationCallbackHandlers[i].Add(handler);
			}
		}

		void RemoveValidationCallbackHandler(ValidationCallback handler)
		{
			for (int i = 0; i < validationCallbackHandlers.GetCapacity(); i++)
			{
				validationCallbackHandlers[i].Remove(handler);
			}
		}

		virtual Array<RHI::Format> GetAvailableDepthStencilFormats() = 0;
		virtual Array<RHI::Format> GetAvailableDepthOnlyFormats() = 0;

		virtual Array<RHI::CommandQueue*> GetHardwareQueues(RHI::HardwareQueueClassMask queueMask) = 0;

		virtual RHI::CommandQueue* GetPrimaryGraphicsQueue() = 0;
		virtual RHI::CommandQueue* GetPrimaryTransferQueue() = 0;

		//! Returns true if RHI was initialized in Offscreen mode, i.e. there was no main window when RHI was initialized. 
		virtual bool IsOffscreenOnly() = 0;

		/// Returns screen size for window based on which monitor it is in. Usually, platformWindowHandle is of type SDL_Window*.
		virtual Vec2i GetScreenSizeForWindow(void* platformWindowHandle) = 0;

        // - Command List -

		virtual RHI::Fence* CreateFence(bool initiallySignalled = false) = 0;
		virtual void DestroyFence(RHI::Fence* fence) = 0;

		virtual RHI::CommandList* AllocateCommandList(RHI::CommandQueue* associatedQueue, 
			CommandListType commandListType = CommandListType::Direct) = 0;

		virtual Array<RHI::CommandList*> AllocateCommandLists(u32 count, RHI::CommandQueue* associatedQueue,
			CommandListType commandListType = CommandListType::Direct) = 0;

		virtual void FreeCommandLists(u32 count, RHI::CommandList** commandLists) = 0;

        // - Resources -

		virtual RHI::DeviceLimits* GetDeviceLimits() = 0;

		virtual RHI::RenderTarget* CreateRenderTarget(const RHI::RenderTargetLayout& rtLayout) = 0;
		virtual void DestroyRenderTarget(RHI::RenderTarget* renderTarget) = 0;

		virtual RHI::RenderTargetBuffer* CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::TextureView*>& imageAttachments, u32 imageIndex = 0) = 0;
		virtual RHI::RenderTargetBuffer* CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::Texture*>& imageAttachments, u32 imageIndex = 0) = 0;
		virtual void DestroyRenderTargetBuffer(RHI::RenderTargetBuffer* renderTargetBuffer) = 0;

		virtual RHI::SwapChain* CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc) = 0;
		virtual void DestroySwapChain(RHI::SwapChain* swapChain) = 0;

		virtual RHI::MemoryHeap* AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc) = 0;
		virtual void FreeMemoryHeap(RHI::MemoryHeap* memoryHeap) = 0;

		virtual void GetBufferMemoryRequirements(const RHI::BufferDescriptor& bufferDesc, RHI::ResourceMemoryRequirements& outRequirements) = 0;
        virtual void GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements) = 0;

		virtual RHI::ResourceMemoryRequirements GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList = nullptr) = 0;

		virtual RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc) = 0;
		virtual RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) = 0;
        virtual void DestroyBuffer(RHI::Buffer* buffer) = 0;

		virtual RHI::TextureView* CreateTextureView(const RHI::TextureViewDescriptor& desc) = 0;
		virtual void DestroyTextureView(RHI::TextureView* textureView) = 0;
        
		virtual RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc) = 0;
		virtual RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) = 0;
        virtual void DestroyTexture(RHI::Texture* texture) = 0;
        
        virtual RHI::Sampler* CreateSampler(const SamplerDescriptor& samplerDesc) = 0;
        virtual void DestroySampler(Sampler* sampler) = 0;

		virtual RHI::ShaderModule* CreateShaderModule(const RHI::ShaderModuleDescriptor& desc) = 0;
		virtual void DestroyShaderModule(RHI::ShaderModule* shaderModule) = 0;

		virtual RHI::ShaderResourceGroup* CreateShaderResourceGroup(const RHI::ShaderResourceGroupLayout& srgLayout) = 0;
		virtual void DestroyShaderResourceGroup(RHI::ShaderResourceGroup* shaderResourceGroup) = 0;

		// - Pipeline State -

		virtual RHI::PipelineState* CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc) = 0;
    	virtual RHI::PipelineState* CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc) = 0;
		virtual void DestroyPipeline(const RHI::PipelineState* pipeline) = 0;

		// - Helper Operations -

		virtual u64 GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member) = 0;
		virtual u64 GetShaderStructMemberSize(const RHI::ShaderStructMember& member) = 0;
		virtual void GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets) = 0;

	protected:

		StaticArray<Array<ValidationCallback>, (SIZE_T)ValidationMessageType::COUNT> validationCallbackHandlers{};
    };

    CORERHI_API extern DynamicRHI* gDynamicRHI;
    
} // namespace CE

