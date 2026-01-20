#pragma once

namespace CE {
	class PlatformWindow;
}

namespace CE::RHI
{

	struct SwapChainDescriptor
	{
		//! @brief Number of images in the swapchain.
		u32 imageCount = 2;

		//! @brief Array of preferred image formats to use.
		Array<RHI::Format> preferredFormats = { RHI::Format::R8G8B8A8_UNORM, RHI::Format::B8G8R8A8_UNORM, };
        
		u32 preferredWidth = 0;
		u32 preferredHeight = 0;

        //! @brief When set to true, the SwapChain can only be used as a color attachment, or as a transfer destination, i.e. SwapChain will become a Write-Only resource.
        bool frameBufferOnly = true;
        
		bool useMailboxMode = false;
	};
    
	class CORERHI_API SwapChain : public RHIResource, public IDeviceObject
	{
	protected:
		SwapChain() : RHIResource(ResourceType::SwapChain), IDeviceObject(DeviceObjectType::SwapChain)
		{}

	public:
        
        bool IsFrameBufferOnly() const { return frameBufferOnly; }

		inline u32 GetWidth() const { return width; }
		inline u32 GetHeight() const { return height; }

		inline u32 GetPreferredWidth() const { return width; }
		inline u32 GetPreferredHeight() const { return height; }

		inline f32 GetAspectRatio() const { return (f32)width / (f32)height; }

		inline RHI::Format GetSwapChainFormat() const { return swapChainColorFormat; }

		virtual PlatformWindow* GetNativeWindow() = 0;
        
        virtual bool AcquireNextImage() = 0;

		virtual void Rebuild() = 0;

	protected:
        
		RHI::Format swapChainColorFormat{};
        bool frameBufferOnly = true;
        
		u32 width = 0;
		u32 height = 0;

		u32 preferredWidth = 0;
		u32 preferredHeight = 0;
	};

} // namespace CE::RHI
