#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    
    ShaderResourceGroup::ShaderResourceGroup(Device* device, const RHI::ShaderResourceGroupDescriptor& srgDescriptor)
        : device(device)
    {
        this->srgLayout = srgDescriptor.layout;
        this->srgType = srgDescriptor.layout.srgType;
        this->name = srgDescriptor.name;
        
        bindingSlotsByVariableName.Clear();
        
        for (const auto& variable : srgLayout.variables)
        {
            int binding = variable.bindingSlot;
            
            bindingSlotsByVariableName[variable.name] = binding;
        }
    }

    ShaderResourceGroup::~ShaderResourceGroup()
    {
        
    }

    bool ShaderResourceGroup::HasVariable(const Name& variableName)
    {
        return bindingSlotsByVariableName.KeyExists(variableName);
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::BufferView bufferView)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, bufferView);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::Texture* texture)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, texture);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::TextureView* textureViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, textureViews);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::Sampler* sampler)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, sampler);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::BufferView* bufferViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, count, bufferViews);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::Texture** textures)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, count, textures);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::TextureView** textureViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, count, textureViews);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::Sampler** samplers)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, count, samplers);
        }
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::BufferView bufferView)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        if (boundBuffersBySlot[imageIndex][bindingSlot].GetSize() == 1 &&
            boundBuffersBySlot[imageIndex][bindingSlot][0].GetBuffer() == bufferView.GetBuffer() &&
            boundBuffersBySlot[imageIndex][bindingSlot][0].GetByteCount() == bufferView.GetByteCount() &&
            boundBuffersBySlot[imageIndex][bindingSlot][0].GetByteOffset() == bufferView.GetByteOffset())
        {
            return true;
        }
        
        boundBuffersBySlot[imageIndex][bindingSlot].Resize(1);
        boundBuffersBySlot[imageIndex][bindingSlot][0] = bufferView;
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::Texture* texture)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        if (boundTexturesBySlot[imageIndex][bindingSlot].GetSize() == 1 &&
            boundTexturesBySlot[imageIndex][bindingSlot][0].resourceType == RHI::ResourceType::Texture &&
            boundTexturesBySlot[imageIndex][bindingSlot][0].texture == (Metal::Texture*)texture)
        {
            return true;
        }
        
        boundTexturesBySlot[imageIndex][bindingSlot].Resize(1);
        boundTexturesBySlot[imageIndex][bindingSlot][0].resourceType = RHI::ResourceType::Texture;
        boundTexturesBySlot[imageIndex][bindingSlot][0].texture = (Metal::Texture*)texture;
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::TextureView* textureView)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        if (boundTexturesBySlot[imageIndex][bindingSlot].GetSize() == 1 &&
            boundTexturesBySlot[imageIndex][bindingSlot][0].resourceType == RHI::ResourceType::TextureView &&
            boundTexturesBySlot[imageIndex][bindingSlot][0].textureView == (Metal::TextureView*)textureView)
        {
            return true;
        }
        
        boundTexturesBySlot[imageIndex][bindingSlot].Resize(1);
        boundTexturesBySlot[imageIndex][bindingSlot][0].resourceType = RHI::ResourceType::TextureView;
        boundTexturesBySlot[imageIndex][bindingSlot][0].textureView = (Metal::TextureView*)textureView;
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::Sampler* sampler)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        if (boundSamplersBySlot[imageIndex][bindingSlot].GetSize() == 1 &&
            boundSamplersBySlot[imageIndex][bindingSlot][0] == sampler)
        {
            return true;
        }
        
        boundSamplersBySlot[imageIndex][bindingSlot].Resize(1);
        boundSamplersBySlot[imageIndex][bindingSlot][0] = (Metal::Sampler*)sampler;
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::BufferView* bufferViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        boundBuffersBySlot[imageIndex][bindingSlot].Clear();
        boundBuffersBySlot[imageIndex][bindingSlot].Reserve(count);
        
        for (int i = 0; i < count; i++)
        {
            boundBuffersBySlot[imageIndex][bindingSlot].Add(bufferViews[i]);
        }
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::Texture** textures)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        boundTexturesBySlot[imageIndex][bindingSlot].Clear();
        boundTexturesBySlot[imageIndex][bindingSlot].Reserve(count);
        
        for (int i = 0; i < count; i++)
        {
            TextureBinding textureBinding{};
            textureBinding.resourceType = RHI::ResourceType::Texture;
            textureBinding.texture = (Metal::Texture*)textures[i];
            
            boundTexturesBySlot[imageIndex][bindingSlot].Add(textureBinding);
        }
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::TextureView** textureViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        boundTexturesBySlot[imageIndex][bindingSlot].Clear();
        boundTexturesBySlot[imageIndex][bindingSlot].Reserve(count);
        
        for (int i = 0; i < count; i++)
        {
            TextureBinding textureBinding{};
            textureBinding.resourceType = RHI::ResourceType::TextureView;
            textureBinding.textureView = (Metal::TextureView*)textureViews[i];
            
            boundTexturesBySlot[imageIndex][bindingSlot].Add(textureBinding);
        }
        
        needsFlush = true;
        
        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::Sampler** samplers)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;
        
        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;
        
        boundSamplersBySlot[imageIndex][bindingSlot].Clear();
        boundSamplersBySlot[imageIndex][bindingSlot].Reserve(count);
        
        for (int i = 0; i < count; i++)
        {
            boundSamplersBySlot[imageIndex][bindingSlot].Add((Metal::Sampler*)samplers[i]);
        }
        
        needsFlush = true;
        
        return true;
    }
    
    bool ShaderResourceGroup::Bind(Name name, u32 firstArrayElement, u32 count, RHI::Texture** textures)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;

        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, firstArrayElement, count, textures);
        }

        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 firstArrayElement, u32 count, RHI::Texture** textures)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;

        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;

        auto& textureList = boundTexturesBySlot[imageIndex][bindingSlot];

        while ((int)textureList.GetSize() < (int)(firstArrayElement + count))
        {
            TextureBinding empty{};
            empty.resourceType = RHI::ResourceType::Texture;
            empty.texture = nullptr;
            textureList.Add(empty);
        }

        for (int j = 0; j < count; j++)
        {
            TextureBinding textureBinding{};
            textureBinding.resourceType = RHI::ResourceType::Texture;
            textureBinding.texture = (Metal::Texture*)textures[j];
            textureList[firstArrayElement + j] = textureBinding;
        }

        needsFlush = true;

        return true;
    }

    bool ShaderResourceGroup::Bind(Name name, u32 firstArrayElement, u32 count, RHI::TextureView** textureViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;

        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            Bind(i, name, firstArrayElement, count, textureViews);
        }

        return true;
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 firstArrayElement, u32 count, RHI::TextureView** textureViews)
    {
        if (!bindingSlotsByVariableName.KeyExists(name))
            return false;

        int bindingSlot = bindingSlotsByVariableName[name];
        if (bindingSlot < 0)
            return false;

        auto& textures = boundTexturesBySlot[imageIndex][bindingSlot];

        // Grow the list to accommodate firstArrayElement + count if needed
        while ((int)textures.GetSize() < (int)(firstArrayElement + count))
        {
            TextureBinding empty{};
            empty.resourceType = RHI::ResourceType::TextureView;
            empty.textureView = nullptr;
            textures.Add(empty);
        }

        for (int j = 0; j < count; j++)
        {
            TextureBinding textureBinding{};
            textureBinding.resourceType = RHI::ResourceType::TextureView;
            textureBinding.textureView = (Metal::TextureView*)textureViews[j];
            textures[firstArrayElement + j] = textureBinding;
        }

        needsFlush = true;

        return true;
    }

    void ShaderResourceGroup::MtlUseResources(id<MTLComputeCommandEncoder> mtlComputeEncoder, int frameIndex)
    {
        for (const auto& [binding, boundBuffers] : boundBuffersBySlot[frameIndex])
        {
            for (const RHI::BufferView& bufferView : boundBuffers)
            {
                if (Metal::Buffer* buffer = (Metal::Buffer*)bufferView.GetBuffer())
                {
                    MTLResourceUsage usage = MTLResourceUsageRead;
                    
                    const auto& variableDesc = srgLayout.FindVariable(binding);
                    if (variableDesc.name.IsValid())
                    {
                        switch (variableDesc.type)
                        {
                            case RHI::ShaderResourceType::ConstantBuffer:
                            case RHI::ShaderResourceType::StructuredBuffer:
                                usage = MTLResourceUsageRead;
                                break;
                            case RHI::ShaderResourceType::RWStructuredBuffer:
                                usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                                break;
                        }
                    }
                    
                    [mtlComputeEncoder useResource:buffer->GetMtlBuffer() usage:usage];
                }
            }
        }
        
        for (const auto& [binding, boundTextures] : boundTexturesBySlot[frameIndex])
        {
            for (const TextureBinding& textureBinding : boundTextures)
            {
                MTLResourceUsage usage = MTLResourceUsageSample;
                
                const auto& variableDesc = srgLayout.FindVariable(binding);
                if (variableDesc.name.IsValid())
                {
                    switch (variableDesc.type)
                    {
                        case RHI::ShaderResourceType::Texture1D:
                        case RHI::ShaderResourceType::Texture2D:
                        case RHI::ShaderResourceType::Texture3D:
                        case RHI::ShaderResourceType::TextureCube:
                        case RHI::ShaderResourceType::Texture2DArray:
                            usage = MTLResourceUsageSample;
                            break;
                        case RHI::ShaderResourceType::RWTexture2D:
                        case RHI::ShaderResourceType::RWTexture3D:
                        case RHI::ShaderResourceType::RWTexture2DArray:
                            usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                            break;
                    }
                }
                
                if (textureBinding.resourceType == RHI::ResourceType::Texture)
                {
                    [mtlComputeEncoder useResource:textureBinding.texture->GetMtlTexture() usage:usage];
                }
                else if (textureBinding.resourceType == RHI::ResourceType::TextureView)
                {
                    [mtlComputeEncoder useResource:textureBinding.textureView->GetMtlTextureView() usage:usage];
                }
            }
        }
    }
    
    void ShaderResourceGroup::MtlUseResources(id<MTLRenderCommandEncoder> mtlRenderEncoder, int frameIndex)
    {
        for (const auto& [binding, boundBuffers] : boundBuffersBySlot[frameIndex])
        {
            for (const RHI::BufferView& bufferView : boundBuffers)
            {
                if (Metal::Buffer* buffer = (Metal::Buffer*)bufferView.GetBuffer())
                {
                    MTLResourceUsage usage = MTLResourceUsageRead;
                    
                    const auto& variableDesc = srgLayout.FindVariable(binding);
                    if (variableDesc.name.IsValid())
                    {
                        switch (variableDesc.type)
                        {
                            case RHI::ShaderResourceType::ConstantBuffer:
                            case RHI::ShaderResourceType::StructuredBuffer:
                                usage = MTLResourceUsageRead;
                                break;
                            case RHI::ShaderResourceType::RWStructuredBuffer:
                                usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                                break;
                        }
                    }
                    
                    [mtlRenderEncoder useResource:buffer->GetMtlBuffer() usage:usage];
                }
            }
        }
        
        for (const auto& [binding, boundTextures] : boundTexturesBySlot[frameIndex])
        {
            for (const TextureBinding& textureBinding : boundTextures)
            {
                MTLResourceUsage usage = MTLResourceUsageSample;
                
                const auto& variableDesc = srgLayout.FindVariable(binding);
                if (variableDesc.name.IsValid())
                {
                    switch (variableDesc.type)
                    {
                        case RHI::ShaderResourceType::Texture1D:
                        case RHI::ShaderResourceType::Texture2D:
                        case RHI::ShaderResourceType::Texture3D:
                        case RHI::ShaderResourceType::TextureCube:
                        case RHI::ShaderResourceType::Texture2DArray:
                            usage = MTLResourceUsageSample;
                            break;
                        case RHI::ShaderResourceType::RWTexture2D:
                        case RHI::ShaderResourceType::RWTexture3D:
                        case RHI::ShaderResourceType::RWTexture2DArray:
                            usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                            break;
                    }
                }
                
                if (textureBinding.resourceType == RHI::ResourceType::Texture)
                {
                    [mtlRenderEncoder useResource:textureBinding.texture->GetMtlTexture() usage:usage];
                }
                else if (textureBinding.resourceType == RHI::ResourceType::TextureView)
                {
                    [mtlRenderEncoder useResource:textureBinding.textureView->GetMtlTextureView() usage:usage];
                }
            }
        }
    }
    
    void ShaderResourceGroup::CommitResources(Metal::CommandList* cmdList)
    {
        if (!cmdList->boundPipeline)
            return;
        
        int setNumber = (int)srgType;
        int frameIndex = cmdList->currentFrameIndex;
        
        if (cmdList->boundPipeline->IsGraphicsPipeline())
        {
            auto graphicsPipeline = (Metal::GraphicsPipeline*)((Metal::PipelineState*)cmdList->boundPipeline)->GetPipeline();
            
            id<MTLArgumentEncoder> vertEncoder = graphicsPipeline->vertEncodersBySetNumber[setNumber];
            if (vertEncoder != nil)
            {
                if (argBufVS[setNumber][frameIndex] == nil)
                {
                    argBufVS[setNumber][frameIndex] = [device->GetHandle() newBufferWithLength:vertEncoder.encodedLength options:MTLResourceStorageModeShared];
                }
                
                [vertEncoder setArgumentBuffer:argBufVS[setNumber][frameIndex] offset:0];
            }
            
            id<MTLArgumentEncoder> fragEncoder = graphicsPipeline->fragEncodersBySetNumber[setNumber];
            if (fragEncoder != nil)
            {
                if (argBufFS[setNumber][frameIndex] == nil)
                {
                    argBufFS[setNumber][frameIndex] = [device->GetHandle() newBufferWithLength:fragEncoder.encodedLength options:MTLResourceStorageModeShared];
                }
                
                [fragEncoder setArgumentBuffer:argBufFS[setNumber][frameIndex] offset:0];
            }
            
            for (const auto& [binding, boundBuffers] : boundBuffersBySlot[frameIndex])
            {
                int idx = 0;
                for (const RHI::BufferView& bufferView : boundBuffers)
                {
                    if (Metal::Buffer* buffer = (Metal::Buffer*)bufferView.GetBuffer())
                    {
                        if (vertEncoder)
                        {
                            [vertEncoder setBuffer:buffer->GetMtlBuffer() offset:bufferView.GetByteOffset() atIndex:(binding + idx)];
                        }
                        if (fragEncoder)
                        {
                            [fragEncoder setBuffer:buffer->GetMtlBuffer() offset:bufferView.GetByteOffset() atIndex:(binding + idx)];
                        }
                    }
                    idx++;
                }
            }
            
            for (const auto& [binding, boundTextures] : boundTexturesBySlot[frameIndex])
            {
                int idx = 0;
                for (const TextureBinding& textureBinding : boundTextures)
                {
                    if (textureBinding.resourceType == RHI::ResourceType::Texture)
                    {
                        if (vertEncoder)
                        {
                            [vertEncoder setTexture:textureBinding.texture->GetMtlTexture() atIndex:(binding + idx)];
                        }
                        if (fragEncoder)
                        {
                            [fragEncoder setTexture:textureBinding.texture->GetMtlTexture() atIndex:(binding + idx)];
                        }
                    }
                    else if (textureBinding.resourceType == RHI::ResourceType::TextureView)
                    {
                        if (vertEncoder)
                        {
                            [vertEncoder setTexture:textureBinding.textureView->GetMtlTextureView() atIndex:(binding + idx)];
                        }
                        if (fragEncoder)
                        {
                            [fragEncoder setTexture:textureBinding.textureView->GetMtlTextureView() atIndex:(binding + idx)];
                        }
                    }
                    idx++;
                }
            }
            
            for (const auto& [binding, boundSamplers] : boundSamplersBySlot[frameIndex])
            {
                int idx = 0;
                for (Metal::Sampler* samplerState : boundSamplers)
                {
                    if (vertEncoder)
                    {
                        [vertEncoder setSamplerState:samplerState->GetMtlSamplerState() atIndex:(binding + idx)];
                    }
                    if (fragEncoder)
                    {
                        [fragEncoder setSamplerState:samplerState->GetMtlSamplerState() atIndex:(binding + idx)];
                    }
                    idx++;
                }
            }
            
            id<MTLRenderCommandEncoder> renderEncoder = cmdList->mtlRenderEncoder;
            
            for (int i = 0; i < (int)RHI::SRGType::COUNT; i++)
            {
                if (argBufVS[i][frameIndex] != nil)
                {
                    [renderEncoder setVertexBuffer:argBufVS[i][frameIndex] offset:0 atIndex:i];
                }
                if (argBufFS[i][frameIndex] != nil)
                {
                    [renderEncoder setFragmentBuffer:argBufFS[i][frameIndex] offset:0 atIndex:i];
                }
            }
        }
        else if (cmdList->boundPipeline->IsComputePipeline())
        {
            
        }
    }

    void ShaderResourceGroup::Compile()
    {
        if (isCompiled)
            return;
        
        isCompiled = false;
    }

    void ShaderResourceGroup::FlushBindings()
    {
        if (needsFlush)
            isCompiled = false;
        
        if (!isCompiled)
        {
            Compile();
        }
    }
    
} // namespace CE::Metal

