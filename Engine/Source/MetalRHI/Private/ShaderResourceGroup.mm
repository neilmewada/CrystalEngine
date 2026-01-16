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
        
        encoder = [function newArgumentEncoderWithBufferIndex:setNumber];
        
        argumentBuffer = [device->GetHandle() newBufferWithLength:encoder.encodedLength options:MTLResourceStorageModeShared];
        
        String bufferLabelString = srgDescriptor.name.GetString();
        NSString* bufferLabel = [[NSString alloc] initWithCString:bufferLabelString.GetCString()];
        
        argumentBuffer.label = bufferLabel;
        
        [encoder setArgumentBuffer:argumentBuffer offset:0];
        
        
    }

    ShaderResourceGroup::~ShaderResourceGroup()
    {
        
    }

    bool ShaderResourceGroup::HasVariable(const Name& variableName)
    {
        
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::BufferView bufferView)
    {
        
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::Texture* texture)
    {
        
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::TextureView* textureViews)
    {
        
    }

    bool ShaderResourceGroup::Bind(Name name, RHI::Sampler* sampler) {
        
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::BufferView* bufferViews) {
        
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::Texture** textures) {
        
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::TextureView** textureViews) {
        
    }

    bool ShaderResourceGroup::Bind(Name name, u32 count, RHI::Sampler** samplers) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::BufferView bufferView) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::Texture* texture) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::TextureView* textureView) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, RHI::Sampler* sampler) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::BufferView* bufferViews) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::Texture** textures) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::TextureView** textureViews) {
        
    }

    bool ShaderResourceGroup::Bind(u32 imageIndex, Name name, u32 count, RHI::Sampler** samplers) {
        
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
                        [encoder setBuffer:buffer->GetMtlBuffer() offset:bufferView.GetByteOffset() atIndex:(binding + idx)];
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
                        [encoder setTexture:textureBinding.texture->GetMtlTexture() atIndex:(binding + idx)];
                    }
                    else if (textureBinding.resourceType == RHI::ResourceType::TextureView)
                    {
                        [encoder setTexture:textureBinding.textureView->GetMtlTextureView() atIndex:(binding + idx)];
                    }
                    idx++;
                }
            }
            
            for (const auto& [binding, boundSamplers] : boundSamplersBySlot[i])
            {
                int idx = 0;
                for (Metal::Sampler* samplerState : boundSamplers)
                {
                    [encoder setSamplerState:samplerState->GetHandle() atIndex:(binding + idx)];
                    idx++;
                }
            }
        }
        
        isCompiled = true;
    }

    void ShaderResourceGroup::FlushBindings()
    {
        if (!isCompiled)
        {
            Compile();
        }
    }
    
} // namespace CE::Metal
