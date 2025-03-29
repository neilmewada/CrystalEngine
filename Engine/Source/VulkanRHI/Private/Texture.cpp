
#include "VulkanRHIPrivate.h"


namespace CE::Vulkan
{
	struct FormatEntry
	{
		RHI::Format rhiFormat = RHI::Format::Undefined;
		VkFormat vkFormat = VK_FORMAT_UNDEFINED;
		u32 numChannels = 0;
		u32 totalBits = 0;
	};

	thread_local static const Array<FormatEntry> formatEntries{
		// RHI::Format, VkFormat, numChannels, totalBytes
		{ RHI::Format::R8_UNORM, VK_FORMAT_R8_UNORM, 1, 8 },
		{ RHI::Format::R8_SNORM, VK_FORMAT_R8_SNORM, 1, 8 },
		{ RHI::Format::R8_SRGB, VK_FORMAT_R8_SRGB, 1, 8 },

		{ RHI::Format::R8G8_SRGB, VK_FORMAT_R8G8_SRGB, 2, 16 },
		{ RHI::Format::R8G8_UNORM, VK_FORMAT_R8G8_UNORM, 2, 16 },

		{ RHI::Format::R16G16_UNORM, VK_FORMAT_R16G16_UNORM, 2, 32 },
		{ RHI::Format::R16G16_SNORM, VK_FORMAT_R16G16_SNORM, 2, 32 },
		{ RHI::Format::R16G16_SINT, VK_FORMAT_R16G16_SINT, 2, 32 },
		{ RHI::Format::R16G16_SFLOAT, VK_FORMAT_R16G16_SFLOAT, 2, 32 },

		{ RHI::Format::R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, 4, 8*8 },
		{ RHI::Format::R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, 4, 16*8 },

		{ RHI::Format::R32G32_UINT, VK_FORMAT_R32G32_UINT, 2, 8*8 },
		{ RHI::Format::R32G32_SINT, VK_FORMAT_R32G32_SINT, 2, 8*8 },
		{ RHI::Format::R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, 2, 8 * 8 },

		{ RHI::Format::R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB, 4, 4 * 8 },
		{ RHI::Format::R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, 4, 4 * 8 },
		{ RHI::Format::R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM, 4, 4 * 8 },

		{ RHI::Format::R5G6B5_UNORM, VK_FORMAT_R5G6B5_UNORM_PACK16, 3, 2 * 8 },
		{ RHI::Format::B5G6R5_UNORM, VK_FORMAT_B5G6R5_UNORM_PACK16, 3, 2 * 8 },

		{ RHI::Format::B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB, 4, 4 * 8 },
		{ RHI::Format::B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, 4, 4 * 8 },
		{ RHI::Format::B8G8R8A8_SNORM, VK_FORMAT_B8G8R8A8_SNORM, 4, 4 * 8 },

		{ RHI::Format::R8G8B8_UNORM, VK_FORMAT_R8G8B8_UNORM, 3, 3 * 8 },
		{ RHI::Format::R8G8B8_SNORM, VK_FORMAT_R8G8B8_SNORM, 3, 3 * 8 },
		{ RHI::Format::R8G8B8_SRGB, VK_FORMAT_R8G8B8_SRGB, 3, 3 * 8 },

		{ RHI::Format::R16_UNORM, VK_FORMAT_R16_UNORM, 1, 2 * 8 },
		{ RHI::Format::R16_SNORM, VK_FORMAT_R16_SNORM, 1, 2 * 8 },
		{ RHI::Format::R16_SFLOAT, VK_FORMAT_R16_SFLOAT, 1, 2 * 8 },
		{ RHI::Format::R32_UINT, VK_FORMAT_R32_UINT, 1, 4 * 8 },
		{ RHI::Format::R32_SINT, VK_FORMAT_R32_SINT, 1, 4 * 8 },
		{ RHI::Format::R32_SFLOAT, VK_FORMAT_R32_SFLOAT, 1, 4 * 8 },
		{ RHI::Format::D32_SFLOAT, VK_FORMAT_D32_SFLOAT, 1, 4 * 8 },
		{ RHI::Format::D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, 1, 32 + 8 },
		{ RHI::Format::D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, 1, 24 + 8 },
		{ RHI::Format::D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, 1, 16 + 8 },
		{ RHI::Format::D16_UNORM, VK_FORMAT_D16_UNORM, 1, 16 },

		{ RHI::Format::B8G8R8_UNORM, VK_FORMAT_B8G8R8_UNORM, 3, 3 * 8 },
		{ RHI::Format::B8G8R8_SNORM, VK_FORMAT_B8G8R8_SNORM, 3, 3 * 8 },
		{ RHI::Format::B8G8R8_SRGB, VK_FORMAT_B8G8R8_SRGB, 3, 3 * 8 },

		{ RHI::Format::BC1_RGB_UNORM, VK_FORMAT_BC1_RGB_UNORM_BLOCK, 3, 4 },
		{ RHI::Format::BC1_RGBA_UNORM, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 4, 4 },
		{ RHI::Format::BC1_RGB_SRGB, VK_FORMAT_BC1_RGB_SRGB_BLOCK, 3, 4 },
		{ RHI::Format::BC1_RGBA_SRGB, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, 4, 4 },

		{ RHI::Format::BC3_SRGB, VK_FORMAT_BC3_SRGB_BLOCK, 4, 8 },
		{ RHI::Format::BC3_UNORM, VK_FORMAT_BC3_UNORM_BLOCK, 4, 8 },
		{ RHI::Format::BC5_UNORM, VK_FORMAT_BC5_UNORM_BLOCK, 2, 8 },

		{ RHI::Format::BC7_UNORM, VK_FORMAT_BC7_UNORM_BLOCK, 4, 8 },
		{ RHI::Format::BC7_SRGB, VK_FORMAT_BC7_SRGB_BLOCK, 4, 8 },
		{ RHI::Format::BC4_UNORM, VK_FORMAT_BC4_UNORM_BLOCK, 1, 4 },
		{ RHI::Format::BC6H_UFLOAT, VK_FORMAT_BC6H_UFLOAT_BLOCK, 4, 8 },
		{ RHI::Format::BC6H_SFLOAT, VK_FORMAT_BC6H_SFLOAT_BLOCK, 4, 8 },
	};

	thread_local static HashMap<RHI::Format, FormatEntry> textureFormatToVkFormatMap{};
	thread_local static HashMap<VkFormat, FormatEntry> vkFormatToTextureFormatMap{};

	static void LoadMappings()
	{
		if (vkFormatToTextureFormatMap.GetSize() == 0)
		{
			for (const auto& entry : formatEntries)
			{
				textureFormatToVkFormatMap[entry.rhiFormat] = entry;
				vkFormatToTextureFormatMap[entry.vkFormat] = entry;
			}
		}
	}

	VkFormat RHIFormatToVkFormat(RHI::Format format)
    {
		LoadMappings();

		if (!textureFormatToVkFormatMap.KeyExists(format))
			return VK_FORMAT_UNDEFINED;

		return textureFormatToVkFormatMap[format].vkFormat;
    }

	bool IsDepthVkFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
		case VK_FORMAT_D32_SFLOAT:
			return true;
		}
		return false;
	}

	bool IsStencilVkFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_S8_UINT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return true;
		}
		return false;
	}

	bool IsFloat16Format(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R16_SFLOAT:
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R16G16B16_SFLOAT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return true;
		}

		return false;
	}

	RHI::Format VkFormatToRHIFormat(VkFormat format)
    {
		LoadMappings();

		if (!vkFormatToTextureFormatMap.KeyExists(format))
			return RHI::Format::Undefined;

		return vkFormatToTextureFormatMap[format].rhiFormat;
    }

	u32 GetNumberOfChannelsForFormat(RHI::Format format)
    {
		LoadMappings();

		for (const auto& entry : formatEntries)
		{
			if (entry.rhiFormat == format)
			{
				return entry.numChannels;
			}
		}

		return 0;
    }

	bool IsDepthFormat(RHI::Format format)
	{
		switch (format)
		{
		case RHI::Format::D16_UNORM_S8_UINT:
		case RHI::Format::D24_UNORM_S8_UINT:
		case RHI::Format::D32_SFLOAT_S8_UINT:
		case RHI::Format::D32_SFLOAT:
			return true;
		}
		return false;
	}

	bool IsDepthStencilFormat(RHI::Format format)
	{
		switch (format)
		{
		case RHI::Format::D16_UNORM_S8_UINT:
		case RHI::Format::D24_UNORM_S8_UINT:
		case RHI::Format::D32_SFLOAT_S8_UINT:
			return true;
		}
		return false;
	}

    void VulkanRHI::GetTextureMemoryRequirements(const RHI::TextureDescriptor& desc, RHI::ResourceMemoryRequirements& outRequirements)
    {
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        switch (desc.dimension)
        {
		case RHI::Dimension::Dim2D:
		case RHI::Dimension::Dim2DArray:
		case RHI::Dimension::DimCUBE:
            imageCI.imageType = VK_IMAGE_TYPE_2D;
            break;
        case RHI::Dimension::Dim3D:
            imageCI.imageType = VK_IMAGE_TYPE_3D;
            break;
        case RHI::Dimension::Dim1D:
            imageCI.imageType = VK_IMAGE_TYPE_1D;
            break;
        }
        
        imageCI.extent.width = desc.width;
        imageCI.extent.height = desc.height;
        imageCI.extent.depth = desc.depth;

		if (desc.defaultHeapType == RHI::MemoryHeapType::Default)
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		else
			imageCI.tiling = VK_IMAGE_TILING_LINEAR;

        imageCI.format = RHIFormatToVkFormat(desc.format);
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        
        if (EnumHasAnyFlags(desc.bindFlags, RHI::TextureBindFlags::ShaderRead))
        {
            imageCI.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderWrite))
		{
			imageCI.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::Color))
        {
            imageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::DepthStencil))
        {
            imageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::Depth))
		{
			imageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
        if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::SubpassInput))
        {
            imageCI.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        
        imageCI.samples = (VkSampleCountFlagBits)desc.sampleCount;
        imageCI.mipLevels = desc.mipLevels;
        imageCI.arrayLayers = 1;
        imageCI.flags = 0;

        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.queueFamilyIndexCount = 0;
        imageCI.pQueueFamilyIndices = nullptr;
        
        VkImage tempImage = nullptr;
        
        if (vkCreateImage(device->GetHandle(), &imageCI, VULKAN_CPU_ALLOCATOR, &tempImage) != VK_SUCCESS)
        {
            return;
        }
        
        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(device->GetHandle(), tempImage, &memoryRequirements);
        
        outRequirements.size = memoryRequirements.size;
        outRequirements.offsetAlignment = memoryRequirements.alignment;
        outRequirements.flags = memoryRequirements.memoryTypeBits;
        
        vkDestroyImage(device->GetHandle(), tempImage, VULKAN_CPU_ALLOCATOR);
    }

	void Texture::Init(const RHI::TextureDescriptor& desc)
	{
		this->name = desc.name;
		this->width = desc.width;
		this->height = desc.height;
		this->depth = desc.depth;
		this->arrayLayers = desc.arrayLayers;

		this->dimension = desc.dimension;
		this->format = desc.format;
		this->mipLevels = desc.mipLevels;
		this->sampleCount = desc.sampleCount;

		VkImageCreateInfo imageCI{};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		switch (desc.dimension)
		{
		case RHI::Dimension::Dim2D:
		case RHI::Dimension::Dim2DArray:
		case RHI::Dimension::DimCUBE:
			imageCI.imageType = VK_IMAGE_TYPE_2D;
			break;
		case RHI::Dimension::Dim3D:
			imageCI.imageType = VK_IMAGE_TYPE_3D;
			break;
		case RHI::Dimension::Dim1D:
			imageCI.imageType = VK_IMAGE_TYPE_1D;
			break;
		}

		imageCI.extent.width = width;
		imageCI.extent.height = height;
		imageCI.extent.depth = depth;

		if (heapType == RHI::MemoryHeapType::Default)
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		else
			imageCI.tiling = VK_IMAGE_TILING_LINEAR;

		imageCI.format = RHIFormatToVkFormat(this->format);
		imageCI.initialLayout = vkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		curImageLayout = vkImageLayout;

		this->vkFormat = imageCI.format;

		bool isDepthFormat = IsDepthVkFormat(vkFormat);
		bool isStencilFormat = IsStencilVkFormat(vkFormat);
		if (isDepthFormat)
			aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (isStencilFormat)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		if (!isDepthFormat && !isStencilFormat)
			aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;

		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderRead))
		{
			imageCI.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::ShaderWrite))
		{
			imageCI.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::Color))
		{
			imageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::DepthStencil))
		{
			imageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::Depth))
		{
			imageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (EnumHasFlag(desc.bindFlags, RHI::TextureBindFlags::SubpassInput))
		{
			imageCI.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}

		imageCI.samples = (VkSampleCountFlagBits)desc.sampleCount;
		imageCI.mipLevels = mipLevels;
		imageCI.arrayLayers = arrayLayers;
		imageCI.flags = 0;

		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.queueFamilyIndexCount = 0;
		imageCI.pQueueFamilyIndices = nullptr;

		if (arrayLayers == 6 && desc.dimension == RHI::Dimension::DimCUBE)
		{
			imageCI.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		if (vkCreateImage(device->GetHandle(), &imageCI, VULKAN_CPU_ALLOCATOR, &image) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create Vulkan Image");
			return;
		}

		device->SetObjectDebugName((uint64_t)image, VK_OBJECT_TYPE_IMAGE, name.GetCString());
	}

	void Texture::AllocateInternal()
	{
		// Custom allocation
		VkMemoryRequirements memRequirements{};
		vkGetImageMemoryRequirements(device->GetHandle(), image, &memRequirements);
		byteSize = memRequirements.size;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device->GetHandle(), &allocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to allocate Vulkan Image Memory");
			return;
		}

		vkBindImageMemory(device->GetHandle(), image, imageMemory, 0);
	}

	void Texture::PostInit(const RHI::TextureDescriptor& desc)
	{
		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.format = vkFormat;
		imageViewCI.image = image;

		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		switch (dimension)
		{
		case RHI::Dimension::Dim2D:
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case Dimension::Dim2DArray:
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
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
		}

		// Subresources allow the view to see only a part of an image
		imageViewCI.subresourceRange.aspectMask = aspectMask;   // Which aspect of the image to view (e.g. color bit for viewing color)
		imageViewCI.subresourceRange.baseMipLevel = 0;          // Start mipmap level to view from
		imageViewCI.subresourceRange.levelCount = mipLevels;    // No. of mipmap levels to view
		imageViewCI.subresourceRange.baseArrayLayer = 0;        // Start array layer to view from
		imageViewCI.subresourceRange.layerCount = arrayLayers;  // No. of array layers to view

		if (vkCreateImageView(device->GetHandle(), &imageViewCI, VULKAN_CPU_ALLOCATOR, &imageView) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create Vulkan Image View");
			return;
		}

		device->SetObjectDebugName((uint64_t)imageView, VK_OBJECT_TYPE_IMAGE_VIEW, name.GetCString());
	}

    Texture::Texture(VulkanDevice* device, const RHI::TextureDescriptor& desc)
        : device(device)
    {
		this->heapType = desc.defaultHeapType;
		Init(desc);
		AllocateInternal();
		PostInit(desc);
    }

	Texture::Texture(VulkanDevice* device, const RHI::TextureDescriptor& desc, const RHI::ResourceMemoryDescriptor& memoryDesc)
		: device(device)
	{
		if (memoryDesc.memoryHeap == nullptr)
		{
			this->heapType = desc.defaultHeapType;
			Init(desc);
			AllocateInternal();
			PostInit(desc);
			return;
		}

		this->heapType = memoryDesc.memoryHeap->GetHeapType();
		Init(desc);

		VkMemoryRequirements memRequirements{};
		vkGetImageMemoryRequirements(device->GetHandle(), image, &memRequirements);
		byteSize = memRequirements.size;

		Vulkan::MemoryHeap* heap = (Vulkan::MemoryHeap*)memoryDesc.memoryHeap;

		auto result = vkBindImageMemory(device->GetHandle(), image, heap->GetHandle(), memoryDesc.memoryOffset);
		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to bind Memory to Texture {}", desc.name);
			return;
		}

		PostInit(desc);
	}

	Texture::Texture(VulkanDevice* device, VkImage image, const RHI::TextureDescriptor& desc, VkImageLayout dstLayout)
		: device(device)
		, importedImage(true)
		, image(image)
	{
		this->format = desc.format;
		this->vkFormat = RHIFormatToVkFormat(desc.format);
		this->name = desc.name;
		this->width = desc.width;
		this->height = desc.height;
		this->depth = desc.depth;
		this->arrayLayers = desc.arrayLayers;

		this->dimension = desc.dimension;
		this->format = desc.format;
		this->mipLevels = desc.mipLevels;
		this->sampleCount = desc.sampleCount;

		bool isDepthFormat = IsDepthVkFormat(vkFormat);
		bool isStencilFormat = IsStencilVkFormat(vkFormat);
		if (isDepthFormat)
			aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (isStencilFormat)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		if (!isDepthFormat && !isStencilFormat)
			aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image;
		switch (dimension)
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
		imageViewCI.format = vkFormat;

		// Allows remapping of RGBA components to other channels
		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Subresources allow the view to view only a part of an image
		imageViewCI.subresourceRange.aspectMask = aspectMask;  // Which aspect of the image to view (e.g. color bit for viewing color)
		imageViewCI.subresourceRange.baseMipLevel = 0;          // Start mipmap level to view from
		imageViewCI.subresourceRange.levelCount = 1;            // No. of mipmap levels to view
		imageViewCI.subresourceRange.baseArrayLayer = 0;        // Start array layer to view from
		imageViewCI.subresourceRange.layerCount = 1;            // No. of array layers to view

		if (vkCreateImageView(device->GetHandle(), &imageViewCI, VULKAN_CPU_ALLOCATOR, &imageView) != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create Vulkan Image View!");
			return;
		}

		if (dstLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			device->TransitionImageLayout(image, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, dstLayout, imageViewCI.subresourceRange, aspectMask);
			vkImageLayout = curImageLayout = dstLayout;
		}
	}

    Texture::~Texture()
    {
        if (imageView != nullptr)
        {
            vkDestroyImageView(device->GetHandle(), imageView, VULKAN_CPU_ALLOCATOR);
            imageView = nullptr;
        }

        if (imageMemory != nullptr)
        {
            vkFreeMemory(device->GetHandle(), imageMemory, VULKAN_CPU_ALLOCATOR);
            imageMemory = nullptr;
        }

		// Don't destroy images that are managed externally
        if (!importedImage && image != nullptr)
        {
            vkDestroyImage(device->GetHandle(), image, VULKAN_CPU_ALLOCATOR);
            image = nullptr;
        }
    }

    u32 Texture::GetNumberOfChannels()
    {
        return GetNumberOfChannelsForFormat(format);
    }

	u32 Texture::GetBitsPerPixel()
	{
		return GetBitsPerPixelForFormat(format);
	}

    void Texture::CopyPixelsFromBuffer(Buffer* srcBuffer)
    {
        auto cmdBuffer = device->BeginSingleUseCommandBuffer();

        VkBuffer srcVkBuffer = srcBuffer->GetBuffer();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
		
        region.imageSubresource.aspectMask = aspectMask;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            depth
        };

        vkCmdCopyBufferToImage(cmdBuffer, srcVkBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        device->EndSingleUseCommandBuffer(cmdBuffer);
        device->SubmitAndWaitSingleUseCommandBuffer(cmdBuffer);
    }

	void Texture::CopyPixelsToBuffer(Buffer* dstBuffer)
	{
		auto cmdBuffer = device->BeginSingleUseCommandBuffer();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = aspectMask;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			depth
		};
        
		vkCmdCopyImageToBuffer(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->GetBuffer(), 1, &region);

		device->EndSingleUseCommandBuffer(cmdBuffer);
		device->SubmitAndWaitSingleUseCommandBuffer(cmdBuffer);
	}

} // namespace CE
