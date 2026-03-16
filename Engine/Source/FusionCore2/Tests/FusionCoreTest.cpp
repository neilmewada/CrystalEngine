#include "FusionCoreTest.h"


namespace RenderingTests
{

	void FusionRenderService::OnStart()
	{
        Super::OnStart();

		PlatformApplication::Get()->AddMessageHandler(this);
	}

	void FusionRenderService::OnShutdown()
	{
		Super::OnShutdown();

        PlatformApplication::Get()->RemoveMessageHandler(this);
	}

	void FusionRenderService::MarkFrameGraphDirty()
	{
        rebuildFrameGraph = recompileFrameGraph = true;
	}

	void FusionRenderService::RenderPrepare()
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

	bool FusionRenderService::BeginRender()
	{
        if (IsEngineRequestingExit())
            return false;

        auto scheduler = RHI::FrameScheduler::Get();

        int imageIndex = scheduler->BeginExecution();

        if (imageIndex >= RHI::Limits::MaxSwapChainImageCount || rebuildFrameGraph || recompileFrameGraph)
        {
			MarkFrameGraphDirty();
            return false;
        }

        curImageIndex = imageIndex;

        return true;
	}

	void FusionRenderService::EndRender()
	{
        auto scheduler = RHI::FrameScheduler::Get();

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

	void FusionRenderService::BuildFrameGraph()
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
                    surface->EmplaceFrameAttachments();
                }
            }

            for (int i = 0; i < application->GetSurfaceCount(); i++)
            {
                if (Ref<FSurface> surface = application->GetSurface(i))
                {
                    surface->EnqueueScopes();
                }
            }
        }
        scheduler->EndFrameGraph();
	}

	void FusionRenderService::CompileFrameGraph()
	{
        recompileFrameGraph = false;

        auto scheduler = RHI::FrameScheduler::Get();

        scheduler->Compile();
	}

	void FusionRenderService::OnWindowRestored(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowDestroyed(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowClosed(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowMinimized(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowCreated(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowExposed(PlatformWindow* window)
	{
        MarkFrameGraphDirty();
	}

	void FusionRenderService::OnWindowShown(PlatformWindow* window)
	{
		MarkFrameGraphDirty();
	}

}

