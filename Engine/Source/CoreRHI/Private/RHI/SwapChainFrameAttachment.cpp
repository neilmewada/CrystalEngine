#include "CoreRHI.h"

namespace CE::RHI
{

    SwapChainFrameAttachment::SwapChainFrameAttachment(AttachmentID id, SwapChain* swapChain)
		: ImageFrameAttachment(id, nullptr)
		, swapChain(swapChain)
    {
        // TODO: Fix this ASAP
        
        for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
        {
            descriptor.width = swapChain->GetWidth();
            descriptor.height = swapChain->GetHeight();
            descriptor.depth = 1;
            descriptor.format = swapChain->GetSwapChainFormat();
            descriptor.sampleCount = 1;
            descriptor.dimension = RHI::Dimension::Dim2D;
            descriptor.arrayLayers = 1;
            descriptor.bindFlags = RHI::TextureBindFlags::Color;
            descriptor.mipLevels = 1;
            descriptor.defaultHeapType = MemoryHeapType::Default;
            
            SetResource(i, nullptr);
        }
        
		/*for (int i = 0; i < swapChain->GetImageCount(); i++)
		{
			Texture* image = swapChain->GetImage(i);
			if (image)
			{
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

			SetResource(i, image);
		}*/
    }

	void SwapChainFrameAttachment::UpdateImage()
	{
		
	}

} // namespace CE::RHI
