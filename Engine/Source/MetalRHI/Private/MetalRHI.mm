
#include "MetalRHI.h"
#include "MetalRHIPrivate.h"

#include "MetalRHI.private.h"

namespace CE::Metal
{
    void MetalRHIModule::StartupModule()
    {
        
    }

    void MetalRHIModule::ShutdownModule()
    {
        
    }

    void MetalRHIModule::RegisterTypes()
    {
        
    }

    void MetalRHI::Initialize()
    {
        device = new Device(this);
        
        device->Initialize();
    }

    void MetalRHI::PostInitialize()
    {
        
    }

    void MetalRHI::PreShutdown()
    {
        if (device)
        {
            device->PreShutdown();
        }
    }

    void MetalRHI::Shutdown()
    {
        if (device)
        {
            device->Shutdown();
        }
        
        delete device; device = nullptr;
    }

    void* MetalRHI::GetNativeHandle()
    {
        return this;
    }

    RHI::Scope* MetalRHI::CreateScope(const ScopeDescriptor& desc)
    {
        // TODO
        return nullptr;
    }

    RHI::FrameGraphCompiler* MetalRHI::CreateFrameGraphCompiler()
    {
        return nullptr;
    }

    RHI::FrameGraphExecuter* MetalRHI::CreateFrameGraphExecuter()
    {
        return nullptr;
    }

    Array<RHI::Format> MetalRHI::GetAvailableDepthStencilFormats()
    {
        return {};
    }

    Array<RHI::Format> MetalRHI::GetAvailableDepthOnlyFormats()
    {
        return {};
    }

    Array<RHI::CommandQueue*> MetalRHI::GetHardwareQueues(RHI::HardwareQueueClassMask queueMask)
    {
        return {};
    }

    RHI::CommandQueue* MetalRHI::GetPrimaryGraphicsQueue()
    {
        return nullptr;
    }

    RHI::CommandQueue* MetalRHI::GetPrimaryTransferQueue()
    {
        return nullptr;
    }

    bool MetalRHI::IsOffscreenOnly()
    {
        return false;
    }

    Vec2i MetalRHI::GetScreenSizeForWindow(void* platformWindowHandle)
    {
        return {};
    }

    RHI::Fence* MetalRHI::CreateFence(bool initiallySignalled)
    {
        return new Metal::Fence(device, initiallySignalled);
    }

    void MetalRHI::DestroyFence(RHI::Fence* fence)
    {
        
    }

    RHI::CommandList* MetalRHI::AllocateCommandList(RHI::CommandQueue* associatedQueue, CommandListType commandListType)
    {
        if (associatedQueue == nullptr)
            return nullptr;
        
        auto queue = (Metal::CommandQueue*)associatedQueue;
        id<MTLCommandQueue> mtlQueue = queue->GetMtlQueue();
        
        return new Metal::CommandList(device, mtlQueue, commandListType);
    }

    Array<RHI::CommandList*> MetalRHI::AllocateCommandLists(u32 count,
                                                            RHI::CommandQueue* associatedQueue, CommandListType commandListType)
    {
        return {};
    }

    void MetalRHI::FreeCommandLists(u32 count, RHI::CommandList** commandLists)
    {
        
    }

    RHI::DeviceLimits* MetalRHI::GetDeviceLimits()
    {
        return device->GetDeviceLimits();
    }

    RHI::RenderTarget* MetalRHI::CreateRenderTarget(const RHI::RenderTargetLayout& rtLayout)
    {
        return nullptr;
    }

    void MetalRHI::DestroyRenderTarget(RHI::RenderTarget* renderTarget)
    {
        return nullptr;
    }

    RHI::RenderTargetBuffer* MetalRHI::CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::TextureView*>& imageAttachments, u32 imageIndex)
    {
        return nullptr;
    }

    RHI::RenderTargetBuffer* MetalRHI::CreateRenderTargetBuffer(RHI::RenderTarget* renderTarget, const Array<RHI::Texture*>& imageAttachments, u32 imageIndex)
    {
        return nullptr;
    }

    void MetalRHI::DestroyRenderTargetBuffer(RHI::RenderTargetBuffer* renderTargetBuffer)
    {
        
    }

    RHI::SwapChain* MetalRHI::CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc)
    {
        return nullptr;
    }

    void MetalRHI::DestroySwapChain(RHI::SwapChain* swapChain)
    {
        return nullptr;
    }

    RHI::MemoryHeap* MetalRHI::AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc)
    {
        return nullptr;
    }

    void MetalRHI::FreeMemoryHeap(RHI::MemoryHeap* memoryHeap)
    {
        
    }

    void MetalRHI::GetBufferMemoryRequirements(const RHI::BufferDescriptor& bufferDesc, RHI::ResourceMemoryRequirements& outRequirements)
    {
        
    }

    void MetalRHI::GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements)
    {
        
    }

    RHI::ResourceMemoryRequirements MetalRHI::GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList)
    {
        return {};
    }

    RHI::Buffer* MetalRHI::CreateBuffer(const RHI::BufferDescriptor& bufferDesc)
    {
        return nullptr;
    }

    RHI::Buffer* MetalRHI::CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc)
    {
        return nullptr;
    }

    void MetalRHI::DestroyBuffer(RHI::Buffer* buffer)
    {
        
    }

    RHI::TextureView* MetalRHI::CreateTextureView(const RHI::TextureViewDescriptor& desc)
    {
        return nullptr;
    }

    void MetalRHI::DestroyTextureView(RHI::TextureView* textureView)
    {
        
    }

    RHI::Texture* MetalRHI::CreateTexture(const RHI::TextureDescriptor& textureDesc)
    {
        return nullptr;
    }

    RHI::Texture* MetalRHI::CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc)
    {
        return nullptr;
    }

    void MetalRHI::DestroyTexture(RHI::Texture* texture)
    {
        
    }

    RHI::Sampler* MetalRHI::CreateSampler(const SamplerDescriptor& samplerDesc)
    {
        return nullptr;
    }

    void MetalRHI::DestroySampler(Sampler* sampler)
    {
        
    }

    RHI::ShaderModule* MetalRHI::CreateShaderModule(const RHI::ShaderModuleDescriptor& desc)
    {
        return new Metal::ShaderModule(device, desc);
    }

    void MetalRHI::DestroyShaderModule(RHI::ShaderModule* shaderModule)
    {
        delete shaderModule;
    }

    RHI::ShaderResourceGroup* MetalRHI::CreateShaderResourceGroup(const RHI::ShaderResourceGroupDescriptor& srgDescriptor)
    {
        return new Metal::ShaderResourceGroup(device, srgDescriptor);
    }

    void MetalRHI::DestroyShaderResourceGroup(RHI::ShaderResourceGroup* shaderResourceGroup)
    {
        
    }

    RHI::PipelineState* MetalRHI::CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc)
    {
        return nullptr;
    }

    RHI::PipelineState* MetalRHI::CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc)
    {
        return nullptr;
    }

    void MetalRHI::DestroyPipeline(const RHI::PipelineState* pipeline)
    {
        
    }

    u64 MetalRHI::GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member)
    {
        
    }

    u64 MetalRHI::GetShaderStructMemberSize(const RHI::ShaderStructMember& member)
    {
        
    }

    void MetalRHI::GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets)
    {
        
    }

}

CE_IMPLEMENT_MODULE(MetalRHI, CE::Metal::MetalRHIModule)
