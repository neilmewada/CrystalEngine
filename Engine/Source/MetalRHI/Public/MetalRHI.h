#pragma once

#include "Core.h"
#include "CoreRHI.h"

namespace CE::Metal
{
    class Device;
    class CommandQueue;
    class SwapChain;

    class MetalRHI : public RHI::DynamicRHI
    {
    public:
        
        virtual ~MetalRHI() = default;
        
        virtual void Initialize() override;
        virtual void PostInitialize() override;
        virtual void PreShutdown() override;
        virtual void Shutdown() override;
        
        void *GetNativeHandle() override;
        
        GraphicsBackend GetGraphicsBackend() override { 
            return RHI::GraphicsBackend::Metal;
        }
        
        RHI::Scope* CreateScope(const ScopeDescriptor &desc) override;
        
        RHI::FrameGraphCompiler* CreateFrameGraphCompiler() override;
        
        RHI::FrameGraphExecuter* CreateFrameGraphExecuter() override;
        
        Array<RHI::Format> GetAvailableDepthStencilFormats() override;
        
        Array<RHI::Format> GetAvailableDepthOnlyFormats() override;
        
        Array<RHI::CommandQueue*> GetHardwareQueues(RHI::HardwareQueueClass queueClass) override;
        
        RHI::CommandQueue* GetPrimaryGraphicsQueue() override;
        
        RHI::CommandQueue* GetPrimaryTransferQueue() override;
        
        bool IsOffscreenOnly() override;
        
        Vec2i GetScreenSizeForWindow(void* platformWindowHandle) override;
        
        RHI::Fence* CreateFence(bool initiallySignalled = false) override;
        
        void DestroyFence(RHI::Fence* fence) override;
        
        RHI::CommandList *AllocateCommandList(RHI::CommandQueue* associatedQueue, CommandListType commandListType = CommandListType::Direct) override;
        
        Array<RHI::CommandList *> AllocateCommandLists(u32 count, RHI::CommandQueue* associatedQueue, CommandListType commandListType = CommandListType::Direct) override;
        
        void FreeCommandLists(u32 count, RHI::CommandList** commandLists) override;
        
        RHI::DeviceLimits* GetDeviceLimits() override;
        
        RHI::RenderTarget* CreateRenderTarget(const RHI::RenderTargetLayout& rtLayout) override;
        
        void DestroyRenderTarget(RHI::RenderTarget* renderTarget) override;
        
        RHI::RenderTargetBuffer* CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::TextureView*>& imageAttachments, u32 imageIndex = 0) override;
        
        RHI::RenderTargetBuffer* CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::Texture*> &imageAttachments, u32 imageIndex = 0) override;
        
        void DestroyRenderTargetBuffer(RHI::RenderTargetBuffer* renderTargetBuffer) override;
        
        RHI::RenderPass* CreateRenderPass(const RHI::RenderPassLayout& rpLayout) override;
        void DestroyRenderPass(RHI::RenderPass* renderPass) override;
        
        RHI::RenderPassFrameBuffer* CreateRenderPassFrameBuffer(const RHI::RenderPassFrameBufferDescriptor& descriptor) override;
        void DestroyRenderPassFrameBuffer(RHI::RenderPassFrameBuffer* frameBuffer) override;
        
        RHI::SwapChain* CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc) override;
        
        void DestroySwapChain(RHI::SwapChain* swapChain) override;
        
        RHI::MemoryHeap* AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc) override;
        
        void FreeMemoryHeap(RHI::MemoryHeap* memoryHeap) override;
        
        void GetBufferMemoryRequirements(const RHI::BufferDescriptor& bufferDesc, RHI::ResourceMemoryRequirements& outRequirements) override;
        
        void GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements) override;
        
        RHI::ResourceMemoryRequirements GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList = nullptr) override;
        
        RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc) override;
        
        RHI::Buffer* CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) override;
        
        void DestroyBuffer(RHI::Buffer* buffer) override;
        
        RHI::TextureView* CreateTextureView(const RHI::TextureViewDescriptor& desc) override;
        
        void DestroyTextureView(RHI::TextureView* textureView) override;
        
        RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc) override;
        
        RHI::Texture* CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc) override;
        
        void DestroyTexture(RHI::Texture* texture) override;
        
        RHI::Sampler* CreateSampler(const SamplerDescriptor& samplerDesc) override;
        
        void DestroySampler(RHI::Sampler* sampler) override;
        
        RHI::ShaderModule* CreateShaderModule(const RHI::ShaderModuleDescriptor& desc) override;
        
        void DestroyShaderModule(RHI::ShaderModule* shaderModule) override;
        
        RHI::ShaderResourceGroup* CreateShaderResourceGroup(const RHI::ShaderResourceGroupDescriptor& srgDescriptor) override;
        
        void DestroyShaderResourceGroup(RHI::ShaderResourceGroup* shaderResourceGroup) override;
        
        RHI::PipelineState* CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc) override;
        
        RHI::PipelineState* CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc) override;
        
        void DestroyPipeline(const RHI::PipelineState* pipeline) override;
        
        u64 GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member) override;
        
        u64 GetShaderStructMemberSize(const RHI::ShaderStructMember& member) override;
        
        void GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets) override;
        
        
    private:
        
        Device* device = nullptr;
    };

    class MetalRHIModule : public CE::Module
    {
    public:
        virtual void StartupModule() override;

        virtual void ShutdownModule() override;

        virtual void RegisterTypes() override;
    };
}
