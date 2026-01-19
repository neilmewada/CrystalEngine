
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

    Array<RHI::CommandQueue*> MetalRHI::GetHardwareQueues(RHI::HardwareQueueClass queueClass)
    {
        if (queueClass == RHI::HardwareQueueClass::Transfer)
            return {device->GetTransferQueue()};
        else if (queueClass == RHI::HardwareQueueClass::Compute)
            return {device->GetComputeQueue()};
        
        return {device->GetPrimaryQueue()};
    }

    RHI::CommandQueue* MetalRHI::GetPrimaryGraphicsQueue()
    {
        return device->GetPrimaryQueue();
    }

    RHI::CommandQueue* MetalRHI::GetPrimaryTransferQueue()
    {
        return device->GetTransferQueue();
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
        return new Metal::RenderTarget(device, rtLayout);
    }

    void MetalRHI::DestroyRenderTarget(RHI::RenderTarget* renderTarget)
    {
        delete renderTarget;
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
        delete renderTargetBuffer;
    }

    RHI::RenderPass* MetalRHI::CreateRenderPass(const RHI::RenderPassLayout& rpLayout)
    {
        return new Metal::RenderPass(device, rpLayout);
    }

    void MetalRHI::DestroyRenderPass(RHI::RenderPass* renderPass)
    {
        delete renderPass;
    }
    
    RHI::RenderPassFrameBuffer* MetalRHI::CreateRenderPassFrameBuffer(const RHI::RenderPassFrameBufferDescriptor& descriptor)
    {
        return new Metal::RenderPassFrameBuffer(device, descriptor);
    }
    
    void MetalRHI::DestroyRenderPassFrameBuffer(RHI::RenderPassFrameBuffer* frameBuffer)
    {
        delete frameBuffer;
    }
    
    RHI::SwapChain* MetalRHI::CreateSwapChain(PlatformWindow* window, const RHI::SwapChainDescriptor& desc)
    {
        return new Metal::SwapChain(device, window, desc);
    }

    void MetalRHI::DestroySwapChain(RHI::SwapChain* swapChain)
    {
        delete swapChain;
    }

    RHI::MemoryHeap* MetalRHI::AllocateMemoryHeap(const RHI::MemoryHeapDescriptor& desc)
    {
        return new Metal::MemoryHeap(device, desc);
    }

    void MetalRHI::FreeMemoryHeap(RHI::MemoryHeap* memoryHeap)
    {
        delete memoryHeap;
    }

    void MetalRHI::GetBufferMemoryRequirements(const RHI::BufferDescriptor& bufferDesc, RHI::ResourceMemoryRequirements& outRequirements)
    {
        // TODO
    }

    void MetalRHI::GetTextureMemoryRequirements(const RHI::TextureDescriptor& textureDesc, RHI::ResourceMemoryRequirements& outRequirements)
    {
        // TODO
    }

    RHI::ResourceMemoryRequirements MetalRHI::GetCombinedResourceRequirements(u32 count, RHI::ResourceMemoryRequirements* requirementsList, u64* outOffsetsList)
    {
        return {};
    }

    RHI::Buffer* MetalRHI::CreateBuffer(const RHI::BufferDescriptor& bufferDesc)
    {
        return new Metal::Buffer(device, bufferDesc);
    }

    RHI::Buffer* MetalRHI::CreateBuffer(const RHI::BufferDescriptor& bufferDesc, const RHI::ResourceMemoryDescriptor& memoryDesc)
    {
        return new Metal::Buffer(device, bufferDesc, memoryDesc);
    }

    void MetalRHI::DestroyBuffer(RHI::Buffer* buffer)
    {
        delete buffer;
    }

    RHI::TextureView* MetalRHI::CreateTextureView(const RHI::TextureViewDescriptor& desc)
    {
        return new Metal::TextureView(device, desc);
    }

    void MetalRHI::DestroyTextureView(RHI::TextureView* textureView)
    {
        delete textureView;
    }

    RHI::Texture* MetalRHI::CreateTexture(const RHI::TextureDescriptor& textureDesc)
    {
        return new Metal::Texture(device, textureDesc);
    }

    RHI::Texture* MetalRHI::CreateTexture(const RHI::TextureDescriptor& textureDesc, const RHI::ResourceMemoryDescriptor& memoryDesc)
    {
        return new Metal::Texture(device, textureDesc, memoryDesc);
    }

    void MetalRHI::DestroyTexture(RHI::Texture* texture)
    {
        delete texture;
    }

    RHI::Sampler* MetalRHI::CreateSampler(const SamplerDescriptor& samplerDesc)
    {
        return new Metal::Sampler(device, samplerDesc);
    }

    void MetalRHI::DestroySampler(RHI::Sampler* sampler)
    {
        delete sampler;
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
        delete shaderResourceGroup;
    }

    RHI::PipelineState* MetalRHI::CreateGraphicsPipeline(const RHI::GraphicsPipelineDescriptor& desc)
    {
        return new Metal::PipelineState(device, desc);
    }

    RHI::PipelineState* MetalRHI::CreateComputePipeline(const RHI::ComputePipelineDescriptor& desc)
    {
        return new Metal::PipelineState(device, desc);
    }

    void MetalRHI::DestroyPipeline(const RHI::PipelineState* pipeline)
    {
        delete pipeline;
    }

    u64 MetalRHI::GetShaderStructMemberAlignment(const RHI::ShaderStructMember& member)
    {
        // TODO
        return 0;
    }

    u64 MetalRHI::GetShaderStructMemberSize(const RHI::ShaderStructMember& member)
    {
        // TODO
        return 0;
    }

    void MetalRHI::GetShaderStructMemberOffsets(const Array<RHI::ShaderStructMember>& members, Array<u64>& outOffsets)
    {
        
    }

}

CE_IMPLEMENT_MODULE(MetalRHI, CE::Metal::MetalRHIModule)
