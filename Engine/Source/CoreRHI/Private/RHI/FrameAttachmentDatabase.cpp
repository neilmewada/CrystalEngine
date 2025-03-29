#include "CoreRHI.h"

namespace CE::RHI
{
    FrameAttachmentDatabase::FrameAttachmentDatabase()
    {
        
    }
    
    FrameAttachmentDatabase::~FrameAttachmentDatabase()
    {
		Clear();
    }

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id, const ImageDescriptor& descriptor)
	{
		if (attachmentsById.KeyExists(id))
			return;
		auto attachment = new ImageFrameAttachment(id, descriptor);
		attachments.Add(attachment);
		attachmentsById[id] = attachment;
	}

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id, const StaticArray<Texture*, Limits::MaxSwapChainImageCount>& frames)
	{
		if (attachmentsById.KeyExists(id))
			return;
		auto attachment = new ImageFrameAttachment(id, frames);
		attachments.Add(attachment);
		attachmentsById[id] = attachment;
	}

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id,
		const StaticArray<TextureView*, Limits::MaxSwapChainImageCount>& frames)
	{
    	if (attachmentsById.KeyExists(id))
    		return;
    	auto attachment = new ImageFrameAttachment(id, frames);
    	attachments.Add(attachment);
    	attachmentsById[id] = attachment;
	}

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id, SwapChain* swapChain)
    {
		if (attachmentsById.KeyExists(id))
			return;
        auto attachment = new SwapChainFrameAttachment(id, swapChain);
        attachments.Add(attachment);
        attachmentsById[id] = attachment;
    }

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id, Texture* image)
	{
		if (attachmentsById.KeyExists(id))
			return;
		auto attachment = new ImageFrameAttachment(id, image);
		attachments.Add(attachment);
		attachmentsById[id] = attachment;
	}

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id, const BufferDescriptor& descriptor)
	{
		if (attachmentsById.KeyExists(id))
			return;
		auto attachment = new BufferFrameAttachment(id, descriptor);
		attachments.Add(attachment);
		attachmentsById[id] = attachment;
	}

	void FrameAttachmentDatabase::EmplaceFrameAttachment(AttachmentID id, Buffer* buffer)
	{
		if (attachmentsById.KeyExists(id))
			return;
		auto attachment = new BufferFrameAttachment(id, buffer);
		attachments.Add(attachment);
		attachmentsById[id] = attachment;
	}

	FrameAttachment* FrameAttachmentDatabase::FindFrameAttachment(AttachmentID id)
	{
		if (attachmentsById.KeyExists(id))
			return attachmentsById[id];
		return nullptr;
	}

    void FrameAttachmentDatabase::Clear()
    {
		for (auto attachment : attachments)
		{
			delete attachment;
		}
		attachments.Clear();
		attachmentsById.Clear();
    }

} // namespace CE::RHI
