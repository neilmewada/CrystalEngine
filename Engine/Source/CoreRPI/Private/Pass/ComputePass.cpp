#include "CoreRPI.h"

namespace CE::RPI
{

    ComputePass::ComputePass()
    {

    }

    ComputePass::~ComputePass()
    {
        
    }

    void ComputePass::ProduceScopes(RHI::FrameScheduler* scheduler)
    {
        Name scopeId = GetScopeId();

        scheduler->BeginScope(scopeId, ScopeOperation::Compute, HardwareQueueClass::Compute);
        {
            auto useAttachment = [&](const PassAttachmentBinding& attachmentBinding)
                {
                    Ptr<PassAttachment> attachment = attachmentBinding.GetOriginalAttachment();
                    if (attachment == nullptr)
                        return;

                    const PassSlot* slot = nullptr;

					for (const PassSlot& passSlot : slots)
					{
						if (passSlot.slotType == attachmentBinding.slotType &&
                            passSlot.name == attachmentBinding.name)
						{
                            slot = &passSlot;
							break;
						}
					}

					if (!slot)
                        return;

                    RHI::ScopeAttachmentAccess attachmentAccess = RHI::ScopeAttachmentAccess::Undefined;
                    switch (slot->slotType)
                    {
                    case PassSlotType::Input:
                        attachmentAccess = RHI::ScopeAttachmentAccess::Read;
	                    break;
                    case PassSlotType::Output:
                        attachmentAccess = RHI::ScopeAttachmentAccess::Write;
	                    break;
                    case PassSlotType::InputOutput:
                        attachmentAccess = RHI::ScopeAttachmentAccess::ReadWrite;
	                    break;
                    default:
                        return;
                    }

                    if (attachment->attachmentDescriptor.type == RHI::AttachmentType::Image)
                    {
	                    RHI::ImageScopeAttachmentDescriptor imageScopeAttachment{};
                        imageScopeAttachment.attachmentId = attachment->attachmentId;
                        imageScopeAttachment.loadStoreAction = slot->loadStoreAction;
                        imageScopeAttachment.multisampleState.sampleCount = attachment->attachmentDescriptor.imageDesc.sampleCount;
                        imageScopeAttachment.shaderInputName = slot->shaderInputName;

                        scheduler->UseAttachment(imageScopeAttachment, slot->attachmentUsage, attachmentAccess);
                    }
                    else if (attachment->attachmentDescriptor.type == RHI::AttachmentType::Buffer)
                    {
	                    RHI::BufferScopeAttachmentDescriptor bufferScopeAttachment{};
                        bufferScopeAttachment.attachmentId = attachment->attachmentId;
                        bufferScopeAttachment.loadStoreAction = slot->loadStoreAction;
                        bufferScopeAttachment.shaderInputName = slot->shaderInputName;

                        scheduler->UseAttachment(bufferScopeAttachment, slot->attachmentUsage, attachmentAccess);
                    }
                };

            for (const PassAttachmentBinding& attachmentBinding : inputBindings)
            {
                useAttachment(attachmentBinding);
            }

            for (const PassAttachmentBinding& attachmentBinding : inputOutputBindings)
            {
                useAttachment(attachmentBinding);
            }

            for (const PassAttachmentBinding& attachmentBinding : outputBindings)
            {
                useAttachment(attachmentBinding);
            }

            auto& attachmentDatabase = scheduler->GetAttachmentDatabase();
            Vec3i dispatchSize = Vec3i(-1, -1, -1);
            bool foundSize = false;

            if (dispatchSizeSource.source.IsValid())
            {
                Ptr<PassAttachment> attachment = renderPipeline->FindAttachment(dispatchSizeSource.source);
                if (attachment != nullptr)
                {
                    RHI::FrameAttachment* sourceAttachment = attachmentDatabase.FindFrameAttachment(attachment->attachmentId);
                    if (sourceAttachment != nullptr)
                    {
                        if (sourceAttachment->IsImageAttachment())
                        {
                            const ImageDescriptor& imageDescriptor = static_cast<ImageFrameAttachment*>(sourceAttachment)->GetImageDescriptor();
                            dispatchSize.x = (int)ceil(imageDescriptor.width * dispatchSizeSource.sizeMultipliers.x);
                            dispatchSize.y = (int)ceil(imageDescriptor.height * dispatchSizeSource.sizeMultipliers.y);
                            dispatchSize.z = (int)ceil(imageDescriptor.depth * dispatchSizeSource.sizeMultipliers.z);
                            foundSize = true;
                        }
                        else if (sourceAttachment->IsBufferAttachment())
                        {

                        }
                    }
                }
            }

            if (!foundSize)
            {
                dispatchSize = dispatchSizeSource.fixedSizes;
            }

            scheduler->SetDispatchGroupCount(dispatchSize.x, dispatchSize.y, dispatchSize.z);

            if (shader)
            {
                scheduler->UsePipeline(shader->GetPipelineState());
            }
            else
            {
                CE_LOG(Error, All, "Compute Pass ({}) does not have compute shader assigned!", GetName());
            }

            // - Use SRGs -

            if (shaderResourceGroup)
            {
	            scheduler->UseShaderResourceGroup(shaderResourceGroup);
            }

            if (viewSrg)
            {
                scheduler->UseShaderResourceGroup(viewSrg);
            }

            if (sceneSrg)
            {
                scheduler->UseShaderResourceGroup(sceneSrg);
            }

            scheduler->UsePassSrgLayout(shader->GetPassSrgLayout());;
        }
        scheduler->EndScope();
    }

    void ComputePass::SetShader(RPI::ComputeShader* shader)
    {
        this->shader = shader;
    }

} // namespace CE

