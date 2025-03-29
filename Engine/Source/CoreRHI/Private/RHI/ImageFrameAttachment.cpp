#include "CoreRHI.h"

namespace CE::RHI
{

    ImageFrameAttachment::ImageFrameAttachment(AttachmentID id, const ImageDescriptor& descriptor)
        : FrameAttachment(id, RHI::AttachmentLifetimeType::Transient), descriptor(descriptor)
    {
        
    }

	ImageFrameAttachment::ImageFrameAttachment(AttachmentID id, Texture* image)
		: FrameAttachment(id, RHI::AttachmentLifetimeType::External)
	{
		if (image)
		{
			SetResource(image);

            descriptor.width = image->GetWidth();
            descriptor.height = image->GetHeight();
            descriptor.depth = image->GetDepth();
            descriptor.format = image->GetFormat();
            descriptor.sampleCount = image->GetSampleCount();
            descriptor.dimension = image->GetDimension();
            descriptor.arrayLayers = image->GetArrayLayerCount();
            descriptor.bindFlags = image->GetBindFlags();
            descriptor.mipLevels = image->GetMipLevelCount();
            descriptor.defaultHeapType = MemoryHeapType::Default;
		}
	}

    ImageFrameAttachment::ImageFrameAttachment(AttachmentID id, const StaticArray<Texture*, Limits::MaxSwapChainImageCount>& frames)
	    : FrameAttachment(id, RHI::AttachmentLifetimeType::External)
    {
	    for (int i = 0; i < frames.GetSize(); ++i)
	    {
            SetResource(i, frames[i]);

            descriptor.width = frames[i]->GetWidth();
            descriptor.height = frames[i]->GetHeight();
            descriptor.depth = frames[i]->GetDepth();
            descriptor.format = frames[i]->GetFormat();
            descriptor.sampleCount = frames[i]->GetSampleCount();
            descriptor.dimension = frames[i]->GetDimension();
            descriptor.arrayLayers = frames[i]->GetArrayLayerCount();
            descriptor.bindFlags = frames[i]->GetBindFlags();
            descriptor.mipLevels = frames[i]->GetMipLevelCount();
            descriptor.defaultHeapType = MemoryHeapType::Default;
	    }
    }

    ImageFrameAttachment::ImageFrameAttachment(AttachmentID id,
	    const StaticArray<TextureView*, Limits::MaxSwapChainImageCount>& frames)
		: FrameAttachment(id, RHI::AttachmentLifetimeType::External)
    {
    	for (int i = 0; i < frames.GetSize(); ++i)
    	{
    		SetResource(i, frames[i]);

    		RHI::Texture* texture = frames[i]->GetTexture();

    		descriptor.width = texture->GetWidth();
    		descriptor.height = texture->GetHeight();
    		descriptor.depth = texture->GetDepth();
    		descriptor.format = frames[i]->GetFormat();
    		descriptor.sampleCount = texture->GetSampleCount();
    		descriptor.dimension = frames[i]->GetDimension();
    		descriptor.arrayLayers = frames[i]->GetArrayLayerCount();
    		descriptor.mipLevels = frames[i]->GetMipLevelCount();
    		descriptor.bindFlags = texture->GetBindFlags();
    		descriptor.defaultHeapType = MemoryHeapType::Default;
    	}
    }

    ImageFrameAttachment::~ImageFrameAttachment()
    {
        
    }
    
} // namespace CE::RHI
