#pragma once

namespace CE::Vulkan
{

    class TextureView : public RHI::TextureView
    {
    public:

        TextureView(Device* device, const RHI::TextureViewDescriptor& desc);

        ~TextureView();

        inline VkImageView GetImageView() const { return imageView; }

    private:

        Device* device = nullptr;
        VkImageView imageView = nullptr;
        VkImageAspectFlags aspectMask{};
    };
    
} // namespace CE::Vulkan
