#pragma once

namespace CE::Metal
{
    class CommandList;
    
    class ShaderResourceGroup : public RHI::ShaderResourceGroup
    {
    public:
        
        ShaderResourceGroup(Device* device, const RHI::ShaderResourceGroupDescriptor& srgDescriptor);

        virtual ~ShaderResourceGroup();
        
        bool HasVariable(const Name& variableName) override;
        
        bool Bind(Name name, RHI::BufferView bufferView) override;
        
        bool Bind(Name name, RHI::Texture* texture) override;
        
        bool Bind(Name name, RHI::TextureView* textureViews) override;
        
        bool Bind(Name name, RHI::Sampler* sampler) override;
        
        bool Bind(Name name, u32 count, RHI::BufferView* bufferViews) override;
        
        bool Bind(Name name, u32 count, RHI::Texture** textures) override;
        
        bool Bind(Name name, u32 count, RHI::TextureView** textureViews) override;
        
        bool Bind(Name name, u32 count, RHI::Sampler** samplers) override;
        
        bool Bind(u32 imageIndex, Name name, RHI::BufferView bufferView) override;
        
        bool Bind(u32 imageIndex, Name name, RHI::Texture* texture) override;
        
        bool Bind(u32 imageIndex, Name name, RHI::TextureView* textureView) override;
        
        bool Bind(u32 imageIndex, Name name, RHI::Sampler* sampler) override;
        
        bool Bind(u32 imageIndex, Name name, u32 count, RHI::BufferView* bufferViews) override;
        
        bool Bind(u32 imageIndex, Name name, u32 count, RHI::Texture** textures) override;
        
        bool Bind(u32 imageIndex, Name name, u32 count, RHI::TextureView** textureViews) override;
        
        bool Bind(u32 imageIndex, Name name, u32 count, RHI::Sampler** samplers) override;
        
        void Compile() override;
        
        void FlushBindings() override;
        
        void MtlUseResources(id<MTLRenderCommandEncoder> mtlRenderEncoder, int frameIndex);
        void MtlUseResources(id<MTLComputeCommandEncoder> mtlComputeEncoder, int frameIndex);
        
        void CommitResources(Metal::CommandList* cmdList);
        
    private:
        
        using BindingSlotId = int;
        
        struct TextureBinding
        {
            RHI::ResourceType resourceType;
            union {
                Metal::Texture* texture;
                Metal::TextureView* textureView;
            };
        };
        
        bool failed = false;
        bool needsFlush = true;
        bool needsMtlCompile = true;
        
        Name name;
        Device* device = nullptr;
        
        HashMap<Name, BindingSlotId> bindingSlotsByVariableName{};
        
        StaticArray<HashMap<BindingSlotId, List<RHI::BufferView>>, RHI::Limits::MaxSwapChainImageCount> boundBuffersBySlot{};
        StaticArray<HashMap<BindingSlotId, List<TextureBinding>>, RHI::Limits::MaxSwapChainImageCount> boundTexturesBySlot{};
        StaticArray<HashMap<BindingSlotId, List<Metal::Sampler*>>, RHI::Limits::MaxSwapChainImageCount> boundSamplersBySlot{};
        
        id<MTLBuffer> argBufVS[8][RHI::Limits::MaxSwapChainImageCount];
        id<MTLBuffer> argBufFS[8][RHI::Limits::MaxSwapChainImageCount];
        id<MTLBuffer> argBufCS[8][RHI::Limits::MaxSwapChainImageCount];
    };
    
} // namespace CE::Metal
