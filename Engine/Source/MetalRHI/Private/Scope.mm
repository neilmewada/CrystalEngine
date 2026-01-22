#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    Scope::Scope(Device* device, const RHI::ScopeDescriptor& desc) : RHI::Scope(desc), device(device)
    {
        renderFinishedFence = gDynamicRHI->CreateFence();
        
        queue = device->GetPrimaryQueue();
        
        for (int i = 0; i < cmdLists.GetSize(); i++)
        {
            cmdLists[i] = new Metal::CommandList(device, queue->GetMtlQueue(), RHI::CommandListType::Direct);
        }
    }
    
    Scope::~Scope()
    {
        delete renderFinishedFence;
        delete renderPass;
        delete frameBuffer;
        
        for (int i = 0; i < cmdLists.GetSize(); i++)
        {
            delete cmdLists[i];
        }
    }
    
    bool Scope::CompileInternal(const FrameGraphCompileRequest& compileRequest)
    {
        Metal::SwapChain* presentSwapChain = nullptr;
        
        if (this->presentSwapChains.NotEmpty())
        {
            presentSwapChain = (Metal::SwapChain*)this->presentSwapChains[0];
        }
        
        if (IsGraphicsPass())
        {
            if (prevSubPass == nullptr && nextSubPass != nullptr)
            {
                // TODO: Add support for subpasses
            }
            else if (!IsSubPass())
            {
                RHI::RenderPassFrameBufferDescriptor fbDesc{};
                RHI::RenderPassLayout rpLayout{};
                rpLayout.subpasses.Add({});
                int attachmentIndex = 0;
                
                for (RHI::ScopeAttachment* scopeAttachment : attachments)
                {
                    if (!scopeAttachment->IsImageAttachment() || scopeAttachment->GetFrameAttachment() == nullptr ||
                        !scopeAttachment->GetFrameAttachment()->IsImageAttachment())
                        continue;
                    
                    if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Shader ||
                        scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Copy)
                        continue;
                    
                    RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)scopeAttachment;
                    ImageFrameAttachment* imageFrameAttachment = (ImageFrameAttachment*)scopeAttachment->GetFrameAttachment();
                    
                    RHI::Format format{};
                    u32 sampleCount = 1;
                    
                    if (imageFrameAttachment->GetLifetimeType() == RHI::AttachmentLifetimeType::Transient)
                    {
                        format = imageFrameAttachment->GetImageDescriptor().format;
                        sampleCount = imageFrameAttachment->GetImageDescriptor().sampleCount;
                        
                        // FIXME: Find a way to create FrameBuffer with transient attachments, because they are allocated after scopes are compiled.
                        //fbDesc.attachments.Add(RHI::RenderPassFrameAttachment(RHI::Limits::MaxSwapChainImageCount, (RHI::Texture**)imageFrameAttachment->GetRe));
                    }
                    else
                    {
                        if (imageFrameAttachment->IsSwapChainAttachment())
                        {
                            SwapChainFrameAttachment* swapChainAttachment = (SwapChainFrameAttachment*)imageFrameAttachment;
                            format = swapChainAttachment->GetSwapChain()->GetSwapChainFormat();
                            sampleCount = 1;
                            
                            fbDesc.attachments.Add(swapChainAttachment->GetSwapChain());
                        }
                        else
                        {
                            RHIResource* resource = imageFrameAttachment->GetResource();
                            if (resource->GetResourceType() == RHI::ResourceType::Texture)
                            {
                                Texture* image = (Texture*)resource;
                                format = image->GetFormat();
                                sampleCount = image->GetSampleCount();
                                
                                fbDesc.attachments.Add(RHI::RenderPassFrameAttachment(RHI::Limits::MaxSwapChainImageCount, (RHI::Texture**)imageFrameAttachment->GetResourceArray()));
                            }
                            else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
                            {
                                TextureView* imageView = (TextureView*)resource;
                                if (imageView->GetTexture() == nullptr)
                                    continue;
                                format = imageView->GetFormat();
                                sampleCount = imageView->GetTexture()->GetSampleCount();
                                
                                fbDesc.attachments.Add(RHI::RenderPassFrameAttachment(RHI::Limits::MaxSwapChainImageCount, (RHI::TextureView**)imageFrameAttachment->GetResourceArray()));
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                    
                    RHI::RenderPassAttachmentLayout attachmentLayout{};
                    attachmentLayout.format = format;
                    attachmentLayout.attachmentUsage = scopeAttachment->GetUsage();
                    attachmentLayout.loadAction = scopeAttachment->GetLoadStoreAction().loadAction;
                    attachmentLayout.storeAction = scopeAttachment->GetLoadStoreAction().storeAction;
                    attachmentLayout.loadActionStencil = scopeAttachment->GetLoadStoreAction().loadActionStencil;
                    attachmentLayout.storeActionStencil = scopeAttachment->GetLoadStoreAction().storeActionStencil;
                    attachmentLayout.attachmentId = scopeAttachment->GetId();
                    attachmentLayout.multisampleState.sampleCount = sampleCount;
                    
                    switch (attachmentLayout.attachmentUsage)
                    {
                        case RHI::ScopeAttachmentUsage::Color:
                            rpLayout.subpasses[0].colorAttachments.Add(attachmentIndex);
                            break;
                        case RHI::ScopeAttachmentUsage::DepthStencil:
                            rpLayout.subpasses[0].depthStencilAttachment.Add(attachmentIndex);
                            break;
                        case RHI::ScopeAttachmentUsage::Resolve:
                            rpLayout.subpasses[0].resolveAttachments.Add(attachmentIndex);
                            break;
                        case RHI::ScopeAttachmentUsage::SubpassInput:
                            rpLayout.subpasses[0].subpassInputAttachments.Add(attachmentIndex);
                            break;
                        default:
                            continue;
                    }
                    
                    rpLayout.attachmentLayouts.Add(attachmentLayout);
                    
                    attachmentIndex++;
                }
                
                renderPass = new Metal::RenderPass(device, rpLayout);
                
                fbDesc.renderPass = renderPass;
                
                frameBuffer = new Metal::RenderPassFrameBuffer(device, fbDesc);
            }
            else
            {
                return false;
            }
        }
        
        return true;
    }
    
}
