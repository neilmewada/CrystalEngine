#include "Engine.h"

namespace CE
{

    CE::RenderPipeline::RenderPipeline()
    {
        if (!IsDefaultInstance())
        {
	        renderPipeline = new RPI::RenderPipeline();
        }
    }

    CE::RenderPipeline::~RenderPipeline()
    {
        delete renderPipeline; renderPipeline = nullptr;
    }

    void CE::RenderPipeline::ConstructPipeline()
    {
	    if (RendererSubsystem* renderer = gEngine->GetSubsystem<RendererSubsystem>())
        {
            renderer->RebuildFrameGraph();
        }

        renderPipeline->isDirty = true;

        renderPipeline->name = GetName();
        renderPipeline->uuid = GetUuid();

        if (targetCamera != nullptr)
        {
            renderPipeline->view = targetCamera->GetRpiView();
        }

        // - Cleanup -
        
        for (int i = (int)renderPipeline->attachments.GetSize() - 1; i >= 0; --i)
        {
            renderPipeline->attachments.RemoveAt(i);
        }

        renderPipeline->passTree->Clear();

		// - Pipeline Output -
        RPI::PassImageAttachmentDesc pipelineOutputDesc{};
        pipelineOutputDesc.name = "PipelineOutput";
        pipelineOutputDesc.lifetime = RHI::AttachmentLifetimeType::External;
        pipelineOutputDesc.imageDescriptor.dimension = RHI::Dimension::Dim2D;
        pipelineOutputDesc.imageDescriptor.bindFlags = RHI::TextureBindFlags::Color | RHI::TextureBindFlags::ShaderRead;

        auto pipelineOutput = new RPI::PassAttachment(pipelineOutputDesc);
        renderPipeline->attachments.Add(pipelineOutput);
    }
    
    void CE::RenderPipeline::Tick()
    {
        if (isDirty)
        {
            isDirty = false;

            ConstructPipeline();
        }
    }

    RHI::DrawListTag CE::RenderPipeline::GetBuiltinDrawListTag(RPI::BuiltinDrawItemTag builtinDrawItemTag)
    {
        return RPI::RPISystem::Get().GetBuiltinDrawListTag(builtinDrawItemTag);
    }

    void CE::RenderPipeline::ApplyChanges()
    {
        MarkDirty();
    }

} // namespace CE
