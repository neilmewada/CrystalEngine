#pragma once

namespace CE::Metal
{

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
        
        
    private:
        
        Device* device = nullptr;
        RHI::ShaderResourceGroupLayout srgLayout;
        
        id<MTLArgumentEncoder> encoder;
        id<MTLBuffer> argumentBuffer;
    };
    
} // namespace CE::Metal
