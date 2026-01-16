#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    
    ShaderResourceGroup::ShaderResourceGroup(Device* device, const RHI::ShaderResourceGroupDescriptor& srgDescriptor)
        : device(device)
        , srgLayout(srgDescriptor.layout)
    {
        auto shader = (Metal::ShaderModule*)srgDescriptor.shader;
        String defaultEntryPoint = shader->GetDefaultEntryPoint().GetString();
        NSString* defaultEntryPointStr = [[NSString alloc] initWithCString:defaultEntryPoint.GetCString()];
        
        id<MTLFunction> function = [shader->GetMtlLibrary() newFunctionWithName:defaultEntryPointStr];
        
        NSUInteger setNumber = (u32)srgLayout.srgType;
        
        encoders = [[NSMutableArray<id<MTLArgumentEncoder>> alloc] init];
        argumentBuffers = [[NSMutableArray<id<MTLBuffer>> alloc] init];
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            [encoders addObject:[function newArgumentEncoderWithBufferIndex:setNumber]];
            [argumentBuffers addObject:[device->GetHandle() newBufferWithLength:encoders[i].encodedLength options:MTLResourceStorageModeShared]];
            
            String bufferLabelString = srgDescriptor.name.GetString();
            NSString* bufferLabel = [[NSString alloc] initWithCString:bufferLabelString.GetCString()];
            
            argumentBuffers[i].label = bufferLabel;
            
            [encoders[i] setArgumentBuffer:argumentBuffers[i] offset:0];
        }
        
        bindingSlotsByVariableName.Clear();
        
        for (const auto& variable : srgLayout.variables)
        {
            int binding = variable.bindingSlot;
            
            bindingSlotsByVariableName[variable.name] = binding;
        }
    }

    ShaderResourceGroup::~ShaderResourceGroup()
    {
        encoders = nil;
        argumentBuffers = nil;
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

    void ShaderResourceGroup::Compile()
    {
        if (isCompiled)
            return;
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            for (const auto& [binding, boundBuffers] : boundBuffersBySlot[i])
            {
                int idx = 0;
                for (const RHI::BufferView& bufferView : boundBuffers)
                {
                    if (Metal::Buffer* buffer = (Metal::Buffer*)bufferView.GetBuffer())
                    {
                        [encoders[i] setBuffer:buffer->GetMtlBuffer() offset:bufferView.GetByteOffset() atIndex:(binding + idx)];
                    }
                    idx++;
                }
            }
            
            for (const auto& [binding, boundTextures] : boundTexturesBySlot[i])
            {
                int idx = 0;
                for (const TextureBinding& textureBinding : boundTextures)
                {
                    if (textureBinding.resourceType == RHI::ResourceType::Texture)
                    {
                        [encoders[i] setTexture:textureBinding.texture->GetMtlTexture() atIndex:(binding + idx)];
                    }
                    else if (textureBinding.resourceType == RHI::ResourceType::TextureView)
                    {
                        [encoders[i] setTexture:textureBinding.textureView->GetMtlTextureView() atIndex:(binding + idx)];
                    }
                    idx++;
                }
            }
            
            for (const auto& [binding, boundSamplers] : boundSamplersBySlot[i])
            {
                int idx = 0;
                for (Metal::Sampler* samplerState : boundSamplers)
                {
                    [encoders[i] setSamplerState:samplerState->GetMtlSamplerState() atIndex:(binding + idx)];
                    idx++;
                }
            }
        }
        
        isCompiled = true;
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
