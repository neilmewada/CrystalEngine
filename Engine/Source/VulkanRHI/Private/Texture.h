#pragma once

namespace CE::Vulkan
{
	class VulkanDevice;

    class Texture : public RHI::Texture
    {
    public:
        Texture(VulkanDevice* device, const RHI::TextureDescriptor& desc);
		Texture(VulkanDevice* device, const RHI::TextureDescriptor& desc, const RHI::ResourceMemoryDescriptor& memoryDesc);

		Texture(VulkanDevice* device, VkImage importedImage, const RHI::TextureDescriptor& imageDesc, 
			VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);

        virtual ~Texture();

        virtual void* GetHandle() override
        {
            return image;
        }

        FORCE_INLINE VkImage GetImage() const
        {
            return image;
        }

        FORCE_INLINE VkImageView GetImageView() const
        {
            return imageView;
        }

		FORCE_INLINE VkImageLayout GetImageLayout() const
		{
			return vkImageLayout;
		}

		FORCE_INLINE VkImageAspectFlags GetAspectMask() const
		{
			return aspectMask;
		}
        
        virtual u32 GetNumberOfChannels() override;

        virtual u32 GetBitsPerPixel() override;

		inline VkImageLayout GetVkImageLayout() const { return vkImageLayout; }
        
        inline VkImageLayout GetCurrentImageLayout() const { return curImageLayout; }

    private:

		void Init(const RHI::TextureDescriptor& desc);
		void AllocateInternal();
		void PostInit(const RHI::TextureDescriptor& desc);
		
		// True if the image is not managed by this Texture object
		bool importedImage = false;

        Atomic<int> curFamilyIndex = -1;

        VulkanDevice* device = nullptr;
        VkImage image = nullptr;
        VkDeviceMemory imageMemory = nullptr;
        VkImageView imageView = nullptr;

		MemoryHeapType heapType{};
        
        VkFormat vkFormat{};
		VkImageLayout vkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageAspectFlags aspectMask{};

		VkImageLayout curImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		friend class RHI::FrameGraph;
		friend class CommandList;
		friend class FrameGraphExecuter;
    };
    
    VkFormat RHIFormatToVkFormat(RHI::Format format);
    RHI::Format VkFormatToRHIFormat(VkFormat format);
    u32 GetNumberOfChannelsForFormat(RHI::Format format);

	bool IsDepthVkFormat(VkFormat format);
	bool IsStencilVkFormat(VkFormat format);
    bool IsFloat16Format(VkFormat format);

	bool IsDepthFormat(RHI::Format format);
	bool IsDepthStencilFormat(RHI::Format format);

} // namespace CE
