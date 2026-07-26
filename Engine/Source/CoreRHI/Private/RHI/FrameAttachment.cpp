#include "CoreRHI.h"

namespace CE::RHI
{

    FrameAttachment::FrameAttachment(const AttachmentID& attachmentId, AttachmentLifetimeType lifetime)
        : attachmentId(attachmentId), lifetime(lifetime)
    {
        
    }

	FrameAttachment::~FrameAttachment()
	{
		if (lifetime == RHI::AttachmentLifetimeType::Transient)
		{
			for (int i = 0; i < resources.GetSize(); i++)
			{
				resources[i] = nullptr;
			}
		}
	}

	void FrameAttachment::SetResource(RHIResource* resource)
	{
		for (int i = 0; i < resources.GetSize(); i++)
		{
			resources[i] = resource;
		}
	}

	void FrameAttachment::SetResource(int frameSlot, RHIResource* resource)
	{
		resources[frameSlot] = resource;
	}

    RHIResource* FrameAttachment::GetResource(int frameSlot)
    {
        return resources[frameSlot];
    }

	RHIResource* FrameAttachment::GetResource()
	{
		return resources[0];
	}
    
} // namespace CE::RHI
