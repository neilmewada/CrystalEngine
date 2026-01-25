#pragma once

namespace CE::RHI
{
    class Texture;

    struct TextureViewDescriptor
    {
        Texture* texture = nullptr;
        RHI::Format format{};
        RHI::Dimension dimension{};

        int baseMipLevel = 0;
        int mipLevelCount = 1;

        int baseArrayLayer = 0;
        int arrayLayerCount = 1;
    };
    
    class CORERHI_API TextureView : public RHI::RHIResource, public IDeviceObject
    {
    public:

        virtual ~TextureView() {}

        inline RHI::Texture* GetTexture() const { return texture; }

        inline RHI::Format GetFormat() const { return format; }

        inline RHI::Dimension GetDimension() const { return dimension; }

        inline int GetBaseMipLevel() const { return baseMipLevel; }
        inline int GetMipLevelCount() const { return mipLevelCount; }

        inline int GetBaseArrayLayer() const { return baseArrayLayer; }
        inline int GetArrayLayerCount() const { return arrayLayerCount; }

        inline u32 GetWidth() const
        {
            if (texture == nullptr)
                return 0;
            return texture->GetWidth(baseMipLevel);
		}

        inline u32 GetHeight() const
        {
            if (texture == nullptr)
                return 0;
            return texture->GetHeight(baseMipLevel);
        }

    protected:

		TextureView() : RHIResource(ResourceType::TextureView), IDeviceObject(DeviceObjectType::TextureView)
        {}

        TextureView(const TextureViewDescriptor& desc) : RHIResource(ResourceType::TextureView), IDeviceObject(DeviceObjectType::TextureView)
            , texture(desc.texture)
            , format(desc.format)
            , dimension(desc.dimension)
            , baseMipLevel(desc.baseMipLevel)
            , mipLevelCount(desc.mipLevelCount)
            , baseArrayLayer(desc.baseArrayLayer)
            , arrayLayerCount(desc.arrayLayerCount)
        {}

        RHI::Texture* texture = nullptr;
        RHI::Format format{};
        RHI::Dimension dimension{};

        int baseMipLevel = 0;
        int mipLevelCount = 1;

        int baseArrayLayer = 0;
        int arrayLayerCount = 1;
    };

} // namespace CE::RHI
