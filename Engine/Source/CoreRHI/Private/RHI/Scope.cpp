#include "CoreRHI.h"

namespace CE::RHI
{

    Scope::Scope(const ScopeDescriptor& desc)
    {
		id = desc.id;
		queueClass = desc.queueClass;
    	operation = desc.operation;
    }

	Scope::~Scope()
	{
		for (RHI::ScopeAttachment* attachment : attachments)
		{
			delete attachment;
		}

		delete passShaderResourceGroup;
		delete subpassShaderResourceGroup;
	}

	void Scope::AddScopeAttachment(ScopeAttachment* attachment)
	{
		attachments.Add(attachment);
		if (attachment->IsBufferAttachment())
			bufferAttachments.Add((BufferScopeAttachment*)attachment);
		if (attachment->IsImageAttachment())
			imageAttachments.Add((ImageScopeAttachment*)attachment);
		if (EnumHasFlag(attachment->access, RHI::ScopeAttachmentAccess::Read))
			readAttachments.Add(attachment);
		if (EnumHasFlag(attachment->access, RHI::ScopeAttachmentAccess::Write))
			writeAttachments.Add(attachment);
	}

	bool Scope::ScopeAttachmentExists(const Name& id)
	{
		for (auto attachment : attachments)
		{
			if (attachment->id == id)
				return true;
		}
		return false;
	}

	ScopeAttachment* Scope::FindScopeAttachment(const Name& id)
	{
		for (auto attachment : attachments)
		{
			if (attachment->id == id)
				return attachment;
		}
		return nullptr;
	}

	bool Scope::Compile(const FrameGraphCompileRequest& compileRequest)
    {
		return CompileInternal(compileRequest);
    }

	bool Scope::UsesAttachment(FrameAttachment* attachment)
	{
		for (auto scopeAttachment : attachments)
		{
			if (scopeAttachment != nullptr && scopeAttachment->attachment == attachment)
				return true;
		}
		return false;
	}

	bool Scope::UsesAttachment(AttachmentID attachmentId)
	{
		for (auto scopeAttachment : attachments)
		{
			if (scopeAttachment != nullptr && scopeAttachment->id == attachmentId)
				return true;
		}
		return false;
	}

	HashMap<ScopeAttachment*, ScopeAttachment*> Scope::FindCommonFrameAttachments(Scope* from, Scope* to)
	{
		if (from == nullptr || to == nullptr)
			return {};

		HashMap<ScopeAttachment*, ScopeAttachment*> result{};

		for (int i = 0; i < from->attachments.GetSize(); i++)
		{
			for (int j = 0; j < to->attachments.GetSize(); j++)
			{
				// Matching frame attachments found
				if (from->attachments[i]->attachment == to->attachments[j]->attachment)
				{
					result[from->attachments[i]] = to->attachments[j];
				}
			}
		}

		return result;
	}

	void Scope::SetShaderResourceGroups(const Array<RHI::ShaderResourceGroup*>& srgs)
	{
		externalShaderResourceGroups = srgs;
	}

	void Scope::AddShaderResourceGroups(RHI::ShaderResourceGroup* srg)
	{
		externalShaderResourceGroups.Add(srg);
	}
    
} // namespace CE::RHI
