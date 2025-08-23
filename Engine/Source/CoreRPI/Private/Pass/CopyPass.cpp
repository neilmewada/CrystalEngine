#include "CoreRPI.h"

namespace CE::RPI
{

    CopyPass::CopyPass()
    {

    }

    CopyPass::~CopyPass()
    {
        
    }

    void CopyPass::ProduceScopes(RHI::FrameScheduler* scheduler)
    {
        Name scopeId = GetScopeId();

        scheduler->BeginScope(scopeId, ScopeOperation::Transfer, HardwareQueueClass::Transfer);
        {
            // - Use Attachments -

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

            if (!perPassSrgLayout.IsEmpty())
            {
                scheduler->UsePassSrgLayout(perPassSrgLayout);
            }
        }
        scheduler->EndScope();
    }


} // namespace CE

