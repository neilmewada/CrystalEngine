#include "FusionTest.h"



namespace WidgetTests
{

#pragma region Renderer System

    void RendererSystem::Init()
    {
        PlatformApplication::Get()->AddMessageHandler(this);

        scheduler = RHI::FrameScheduler::Get();
    }

    void RendererSystem::Shutdown()
    {
        PlatformApplication::Get()->RemoveMessageHandler(this);
    }

    void RendererSystem::Render()
    {
        FusionApplication* app = FusionApplication::TryGet();

        int submittedImageIndex = -1;

        if (app)
        {
            app->Tick();
        }

        if (IsEngineRequestingExit())
        {
            return;
        }

        if (rebuildFrameGraph)
        {
            rebuildFrameGraph = false;
            recompileFrameGraph = true;

            BuildFrameGraph();
            submittedImageIndex = curImageIndex;
        }

        if (recompileFrameGraph)
        {
            recompileFrameGraph = false;

            CompileFrameGraph();
        }

        if (IsEngineRequestingExit())
        {
            return;
        }

        auto scheduler = FrameScheduler::Get();

        if (rebuildFrameGraph || recompileFrameGraph)
        {
            RebuildFrameGraph();
            return;
        }

        int imageIndex = scheduler->BeginExecution();

        if (imageIndex >= RHI::Limits::MaxSwapChainImageCount || rebuildFrameGraph || recompileFrameGraph)
        {
            RebuildFrameGraph();
            return;
        }

        curImageIndex = imageIndex;

        // ---------------------------------------------------------
        // - Enqueue draw packets to views

        if (submittedImageIndex != curImageIndex)
        {
            RPI::RPISystem::Get().SimulationTick(curImageIndex);
            RPI::RPISystem::Get().RenderTick(curImageIndex);
        }

        // ---------------------------------------------------------
        // - Submit draw lists to scopes for execution

        drawList.Shutdown();

        RHI::DrawListMask drawListMask{};
        HashSet<RHI::DrawListTag> drawListTags{};

        // - Setup draw list mask

        if (app)
        {
            app->UpdateDrawListMask(drawListMask);
        }

        // - Enqueue additional draw packets

        for (int i = 0; i < drawListMask.GetSize(); ++i)
        {
            if (drawListMask.Test(i))
            {
                drawListTags.Add((u8)i);
            }
        }

        drawList.Init(drawListMask);

        if (app)
        {
            app->EnqueueDrawPackets(drawList, curImageIndex);
        }

        drawList.Finalize();

        // - Set scope draw lists

        if (app) // FWidget Scopes & DrawLists
        {
            app->FlushDrawPackets(drawList, curImageIndex);
        }


        scheduler->EndExecution();
    }

    void RendererSystem::RebuildFrameGraph()
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::BuildFrameGraph()
    {
        rebuildFrameGraph = false;
        recompileFrameGraph = true;

        RPI::RPISystem::Get().SimulationTick(curImageIndex);
        RPI::RPISystem::Get().RenderTick(curImageIndex);

        auto scheduler = RHI::FrameScheduler::Get();

        RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

        scheduler->BeginFrameGraph();
        {
            auto app = FusionApplication::TryGet();

            if (app)
            {
                app->EmplaceFrameAttachments();

                app->EnqueueScopes();
            }
        }
        scheduler->EndFrameGraph();
    }

    void RendererSystem::CompileFrameGraph()
    {
        recompileFrameGraph = false;

        auto scheduler = RHI::FrameScheduler::Get();

        scheduler->Compile();

        RHI::TransientMemoryPool* pool = scheduler->GetTransientPool();
        RHI::MemoryHeap* imageHeap = pool->GetImagePool();
    }


    void RendererSystem::OnWindowRestored(PlatformWindow* window)
    {
        RebuildFrameGraph();
    }

    void RendererSystem::OnWindowDestroyed(PlatformWindow* window)
    {
        RebuildFrameGraph();
    }

    void RendererSystem::OnWindowClosed(PlatformWindow* window)
    {
        RebuildFrameGraph();
    }

    void RendererSystem::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
    {
        RebuildFrameGraph();
    }

    void RendererSystem::OnWindowMinimized(PlatformWindow* window)
    {
        RebuildFrameGraph();
    }

    void RendererSystem::OnWindowCreated(PlatformWindow* window)
    {
        RebuildFrameGraph();
    }

    void RendererSystem::OnWindowExposed(PlatformWindow* window)
    {
        auto id = window->GetWindowId();
        Vec2i windowSize = window->GetWindowSize();

        if (!windowSizesById.KeyExists(id) || windowSize != windowSizesById[id])
        {
            RebuildFrameGraph();

            scheduler->ResetFramesInFlight();
        }

        windowSizesById[id] = windowSize;
    }

#pragma endregion

    void FusionTestWindow::Construct()
    {
        Super::Construct();

        Title("Fusion Test");

        Content(
            FNew(FDockspace)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .FillRatio(1.0f)
        );
    }

}

