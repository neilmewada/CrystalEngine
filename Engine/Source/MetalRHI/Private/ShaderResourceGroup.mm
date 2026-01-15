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

    void ShaderResourceGroup::Compile() {
        
    }

    void ShaderResourceGroup::FlushBindings() {
        
    }

    
} // namespace CE::Metal
