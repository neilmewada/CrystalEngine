#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

    TextureView::TextureView(Device* device, const RHI::TextureViewDescriptor& desc) : RHI::TextureView(desc), device(device)
    {
        VkImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        
        Vulkan::Texture* texture = (Vulkan::Texture*)desc.texture;
        
        imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCI.format = RHIFormatToVkFormat(desc.format);
        imageViewCI.image = texture->GetImage();

        switch (desc.dimension)
        {
        case RHI::Dimension::Dim2D:
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case RHI::Dimension::DimCUBE:
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        case RHI::Dimension::Dim3D:
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case RHI::Dimension::Dim1D:
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
            break;
		case Dimension::Dim2DArray:
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			break;
        }

        bool isDepthFormat = IsDepthVkFormat(imageViewCI.format);
        bool isStencilFormat = IsStencilVkFormat(imageViewCI.format);
        aspectMask = 0;
        if (isDepthFormat)
            aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (isStencilFormat)
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        if (!isDepthFormat && !isStencilFormat)
            aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;

        // Subresources allow the view to see only a part of an image
        imageViewCI.subresourceRange.aspectMask = aspectMask;   // Which aspect of the image to view (e.g. color bit for viewing color)
        imageViewCI.subresourceRange.baseMipLevel = desc.baseMipLevel;          // Start mipmap level to view from
        imageViewCI.subresourceRange.levelCount = desc.mipLevelCount;    // No. of mipmap levels to view
        imageViewCI.subresourceRange.baseArrayLayer = desc.baseArrayLayer;        // Start array layer to view from
        imageViewCI.subresourceRange.layerCount = desc.arrayLayerCount;  // No. of array layers to view

        if (vkCreateImageView(device->GetHandle(), &imageViewCI, VULKAN_CPU_ALLOCATOR, &imageView) != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create Vulkan Image View");
            return;
        }
    }

    TextureView::~TextureView()
    {
        if (imageView)
        {
            vkDestroyImageView(device->GetHandle(), imageView, VULKAN_CPU_ALLOCATOR);
            imageView = nullptr;
        }
    }
    
} // namespace CE::Vulkan
