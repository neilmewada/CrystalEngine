#include "CoreRHI.h"

namespace CE::RHI
{

	void FrameGraphCompiler::Compile(const FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		// 1. Compile scope
		CompileScopes(compileRequest);

		// 2. Compile transient attachments (also allocates heap memory)
		CompileTransientAttachments(compileRequest);

		// Platform specific compilation
		CompileInternal(compileRequest);
	}

	void FrameGraphCompiler::CompileScopes(const FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		CompileScopesInternal(compileRequest);
	}

	void FrameGraphCompiler::CompileTransientAttachments(const FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		FrameGraph* frameGraph = compileRequest.frameGraph;
		TransientAttachmentPool* pool = compileRequest.transientPool;
		const u32 frameSlot = compileRequest.frameSlot;

		const Array<RHI::FrameAttachment*>& attachments = frameGraph->attachmentDatabase.GetAttachments();

		// TODO: Implement memory aliasing later

		ResourceMemoryRequirements bufferReq = {};
		u64 bufferOffset = 0;
		ResourceMemoryRequirements imageReq = {};
		u64 imageOffset = 0;
		Array<u64> attachmentOffsets{};

		for (int imageIdx = 0; imageIdx < compileRequest.numFramesInFlight; imageIdx++)
		{
			for (int i = 0; i < attachments.GetSize(); i++)
			{
				auto attachment = attachments[i];
				if (attachment->GetLifetimeType() != RHI::AttachmentLifetimeType::Transient)
					continue;

				// Reset the resource pointer, we will be recreating the buffer/image anyway.
				attachment->SetResource(nullptr);

				if (attachment->IsBufferAttachment())
				{
					auto bufferAttachment = (RHI::BufferFrameAttachment*)attachment;
					const auto& desc = bufferAttachment->GetBufferDescriptor();
					ResourceMemoryRequirements req{};
					RHI::gDynamicRHI->GetBufferMemoryRequirements(desc, req);
					if (bufferOffset > 0)
						bufferOffset = Memory::AlignUp(bufferOffset, req.offsetAlignment);
					attachmentOffsets.Add(bufferOffset);
					bufferReq.size = bufferOffset + req.size;
					bufferOffset += req.size;
					if (bufferReq.flags == 0)
						bufferReq.flags = req.flags;
					else
						bufferReq.flags &= req.flags;
				}
				else if (attachment->IsImageAttachment())
				{
					auto imageAttachment = (RHI::ImageFrameAttachment*)attachment;
					const auto& desc = imageAttachment->GetImageDescriptor();
					ResourceMemoryRequirements req{};
					RHI::gDynamicRHI->GetTextureMemoryRequirements(desc, req);
					if (imageOffset > 0)
						imageOffset = Memory::AlignUp(imageOffset, req.offsetAlignment);
					attachmentOffsets.Add(imageOffset);
					imageReq.size = imageOffset + req.size;
					imageOffset += req.size;
					if (imageReq.flags == 0)
						imageReq.flags = req.flags;
					else
						imageReq.flags &= req.flags;
				}
			}
		}

		bool bufferPoolRecreated = false;
		bool imagePoolRecreated = false;
		TransientMemoryAllocation allocationInfo{};
		allocationInfo.bufferPool = bufferReq;
		allocationInfo.imagePool = imageReq;

		// Allocate aliased memory pool
		pool->AllocateMemoryPool(allocationInfo, &bufferPoolRecreated, &imagePoolRecreated, compileRequest.shrinkPool);

		int attachmentIdx = 0;

		for (int imageIdx = 0; imageIdx < compileRequest.numFramesInFlight; imageIdx++)
		{
			// Create & bind buffers & images
			for (int i = 0; i < attachments.GetSize(); i++)
			{
				auto attachment = attachments[i];
				if (attachment->GetLifetimeType() != RHI::AttachmentLifetimeType::Transient)
					continue;

				if (attachment->IsBufferAttachment())
				{
					auto bufferAttachment = (RHI::BufferFrameAttachment*)attachment;
					const auto& desc = bufferAttachment->GetBufferDescriptor();
					RHI::Buffer* buffer = pool->AllocateBuffer(desc, attachmentOffsets[attachmentIdx++]);
					bufferAttachment->SetResource(imageIdx, buffer);
				}
				else if (attachment->IsImageAttachment())
				{
					auto imageAttachment = (RHI::ImageFrameAttachment*)attachment;
					const auto& desc = imageAttachment->GetImageDescriptor();
					RHI::Texture* image = pool->AllocateImage(desc, attachmentOffsets[attachmentIdx++]);
					imageAttachment->SetResource(imageIdx, image);
				}
			}
		}
	}

} // namespace CE::RHI
