#include "FusionCoreTest.h"


namespace RenderingTests
{

	void FusionRendererService::OnStart()
	{
        Super::OnStart();

		PlatformApplication::Get()->AddMessageHandler(this);
	}

	void FusionRendererService::OnShutdown()
	{
		Super::OnShutdown();

        PlatformApplication::Get()->RemoveMessageHandler(this);
	}

	void FusionRendererService::MarkFrameGraphDirty()
	{
        rebuildFrameGraph = recompileFrameGraph = true;
	}

	void FusionRendererService::RenderPrepare()
	{
        if (IsEngineRequestingExit())
            return;

		if (rebuildFrameGraph)
        {
            BuildFrameGraph();
            CompileFrameGraph();
            //submittedImageIndex = curImageIndex;
        }

		rebuildFrameGraph = false;
		recompileFrameGraph = false;
	}

	void FusionRendererService::RenderFrame()
	{
        if (IsEngineRequestingExit())
            return;

        auto scheduler = RHI::FrameScheduler::Get();

        int imageIndex = scheduler->BeginExecution();

        if (imageIndex >= RHI::Limits::MaxSwapChainImageCount || rebuildFrameGraph || recompileFrameGraph)
        {
			MarkFrameGraphDirty();
            return;
        }

        curImageIndex = imageIndex;

        // ---------------------------------------------------------
        // - Enqueue draw packets to views

        if (frameGraphBuildSubmissionIndex != curImageIndex)
        {
            RPI::RPISystem::Get().SimulationTick(curImageIndex);
            RPI::RPISystem::Get().RenderTick(curImageIndex);
        }

        frameGraphBuildSubmissionIndex = -1;

        // ---------------------------------------------------------
        // - Submit draw lists to scopes for execution

        drawList.Shutdown();

        RHI::DrawListMask drawListMask{};
        HashSet<RHI::DrawListTag> drawListTags{};

        // - Setup draw list mask

		UpdateDrawListMask(drawListMask);

        for (int i = 0; i < drawListMask.GetSize(); ++i)
        {
            if (drawListMask.Test(i))
            {
                drawListTags.Add((u8)i);
            }
        }

        // - Enqueue Fusion draw packets

        drawList.Init(drawListMask);

        for (int i = 0; i < application->GetSurfaceCount(); i++)
        {
            if (Ref<FSurface> surface = application->GetSurface(i))
            {
                // TODO: Implement below method
				//surface->FlushDrawPackets(drawList, curImageIndex);
            }
        }

        drawList.Finalize();

        scheduler->EndExecution();
	}

	void FusionRendererService::BuildFrameGraph()
	{
        rebuildFrameGraph = false;
        recompileFrameGraph = true;

        RPI::RPISystem::Get().SimulationTick(curImageIndex);
        RPI::RPISystem::Get().RenderTick(curImageIndex);
        frameGraphBuildSubmissionIndex = curImageIndex;

        auto scheduler = RHI::FrameScheduler::Get();

        RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

        scheduler->BeginFrameGraph();
        {
            for (int i = 0; i < application->GetSurfaceCount(); i++)
            {
                if (Ref<FSurface> surface = application->GetSurface(i))
                {
                    
                }
            }

        }
        scheduler->EndFrameGraph();
	}

	void FusionRendererService::CompileFrameGraph()
	{
        recompileFrameGraph = false;

        auto scheduler = RHI::FrameScheduler::Get();

        scheduler->Compile();
	}

	void FusionRendererService::OnWindowRestored(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowDestroyed(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowClosed(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowMinimized(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowCreated(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowExposed(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRendererService::OnWindowShown(PlatformWindow* window)
	{
		MarkFrameGraphDirty();
	}

}

