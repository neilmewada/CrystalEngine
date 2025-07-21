#pragma once

namespace CE::RPI
{
    struct TextureDescriptor
    {
        RHI::TextureDescriptor texture{};
        RHI::SamplerDescriptor samplerDesc{};
        BinaryBlob* source = nullptr;
    };
    
    class CORERPI_API Texture
    {
    public:
        Texture(const TextureDescriptor& desc);
        Texture(RHI::Texture* texture, const RHI::SamplerDescriptor& samplerDesc = {});
        Texture(RHI::TextureView* textureView, const RHI::SamplerDescriptor& samplerDesc = {});
        Texture(const CMImage& sourceImage, const RHI::SamplerDescriptor& samplerDesc = {});
        Texture(const Name& imageName, const Array<CMImage>& sourceImageMips, const RHI::SamplerDescriptor& samplerDesc = {});

        virtual ~Texture();

        inline RHI::Texture* GetRhiTexture() const { return texture; }
        inline RHI::Sampler* GetSamplerState() const { return samplerState; }
        inline RHI::TextureView* GetRhiTextureView() const { return textureView; }

        RHI::TextureView* GetOrCreateTextureView();

        void UploadData(u8* src, u64 dataSize);

        u32 GetWidth() const { return width; }

        u32 GetHeight() const { return height; }

        u32 GetDepth() const { return depth; }

        void TransitionResourceTo(RHI::ResourceState fromState, RHI::ResourceState toState);

    protected:

        RHI::Texture* texture = nullptr;
        RHI::TextureView* textureView = nullptr;
        RHI::Sampler* samplerState = nullptr;

        u32 width = 0;
        u32 height = 0;
        u32 depth = 1;
    };

} // namespace CE::RPI
