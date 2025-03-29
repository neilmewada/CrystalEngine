#include "CoreRPI.h"

namespace CE::RPI
{

	RenderPipeline::RenderPipeline()
	{
		passTree = CreateObject<PassTree>(GetTransient(MODULE_NAME), "PassTree");
		passTree->rootPass->renderPipeline = this;
	}

    RenderPipeline::~RenderPipeline()
    {
		if (passTree)
		{
			passTree->BeginDestroy();
			passTree = nullptr;
		}
    }

    Ptr<PassAttachment> RenderPipeline::FindAttachment(Name name)
    {
		for (PassAttachment* passAttachment : attachments)
		{
			if (passAttachment->name == name)
			{
				return passAttachment;
			}
		}
		return nullptr;
    }

    PassAttachment* RenderPipeline::AddAttachment(const RPI::PassImageAttachmentDesc& imageDesc)
    {
		auto attachment = new PassAttachment(imageDesc);
		attachments.Add(attachment);
		return attachment;
    }

    PassAttachment* RenderPipeline::AddAttachment(const RPI::PassBufferAttachmentDesc& bufferDesc)
    {
		auto attachment = new PassAttachment(bufferDesc);
		attachments.Add(attachment);
		return attachment;
    }
	
    void RenderPipeline::ImportScopeProducers(RHI::FrameScheduler* scheduler)
    {
		std::function<void(Pass*)> addScopesRecursively = [&](Pass* currentPass)
			{
				if (currentPass == nullptr)
					return;
				
				if (currentPass->IsParentPass())
				{
					auto parentPass = (ParentPass*)currentPass;

					for (Pass* pass : parentPass->passes)
					{
						addScopesRecursively(pass);
					}
					return;
				}

				scheduler->AddScopeProducer(currentPass);
			};

		addScopesRecursively(passTree->rootPass);

		const auto& scopeProducers = scheduler->GetScopeProducers();

		for (int i = 0; i < scopeProducers.GetSize(); i++)
		{
			RHI::IScopeProducer* scopeProducer = scopeProducers[i];
			scopeProducer->EmplaceAttachments(scheduler);
		}

		for (int i = 0; i < scopeProducers.GetSize(); i++)
		{
			RHI::IScopeProducer* scopeProducer = scopeProducers[i];
			scopeProducer->ProduceScopes(scheduler);
		}
    }

    void RenderPipeline::ApplyShaderLayout(RPI::ShaderCollection* shaderCollection)
    {
		if (!passTree)
			return;

		passTree->IterateRecursively([shaderCollection](Pass* pass)
			{
				if (pass->IsOfType<RasterPass>())
				{
					RHI::DrawListTag drawListTag = pass->GetDrawListTag();
					if (!drawListTag.IsValid())
						return;

					if (!shaderCollection)
						return;

					RPI::Shader* shader = shaderCollection->GetShader(drawListTag);
					if (!shader)
						return;

					pass->perPassSrgLayout = shader->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerPass);
				}
			});
    }

} // namespace CE::RPI
