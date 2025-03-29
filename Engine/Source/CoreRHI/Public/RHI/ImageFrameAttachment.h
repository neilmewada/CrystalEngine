#pragma once

namespace CE::RHI
{

    class CORERHI_API ImageFrameAttachment : public FrameAttachment
    {
    public:

        //! Create a transient image attachment
        ImageFrameAttachment(AttachmentID id, const ImageDescriptor& imageDesc);

		//! Create an external image attachment 
		ImageFrameAttachment(AttachmentID id, Texture* image);

        //! Create an external FrameBuffer attachment
        ImageFrameAttachment(AttachmentID id, const StaticArray<Texture*, Limits::MaxSwapChainImageCount>& frames);

    	//! Create an external FrameBuffer attachment
    	ImageFrameAttachment(AttachmentID id, const StaticArray<TextureView*, Limits::MaxSwapChainImageCount>& frames);

        virtual ~ImageFrameAttachment();

		virtual bool IsBufferAttachment() const override final { return false; }
		virtual bool IsImageAttachment() const override final { return true; }

		inline const ImageDescriptor& GetImageDescriptor() const { return descriptor; }
        
	protected:
        ImageDescriptor descriptor{};
        
    };
    
} // namespace CE::RHI
