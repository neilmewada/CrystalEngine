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

	FusionTestWindow::FusionTestWindow()
	{
        m_DockspaceClass = MajorDockspace::StaticClass();
	}

    void FusionTestWindow::Construct()
    {
        Super::Construct();

        for (int i = 1; i <= 3; i++)
        {
            Ref<MinorDockspace> minorDockspace;

            dockspace->AddDockWindow(
                FNew(FDockWindow)
                .CanBeUndocked(i != 1)
                .AllowedDockspaces(FDockspaceFilter().WithDockTypeMask(FDockTypeMask::Major))
                .Title(String::Format("Major {}", i))
                .Background(Color::RGBA(26, 26, 26))
                .Child(
                    FNew(FVerticalStack)
                    .ContentHAlign(HAlign::Fill)
                    .ContentVAlign(VAlign::Top)
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Fill)
                    (
                        FNew(FStyledWidget)
                        .Background(Color::RGBA(36, 36, 36))
                        .HAlign(HAlign::Fill)
                        .Height(40),

                        FNew(FStyledWidget)
                        .Background(Color::RGBA(26, 26, 26))
                        .HAlign(HAlign::Fill)
                        .Height(1.0f),

                        FAssignNew(MinorDockspace, minorDockspace)
                        .HAlign(HAlign::Fill)
                        .FillRatio(1.0f)
                    )
                )
                .Name(String::Format("Major{}", i))
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                .As<FDockWindow>()
            );

            for (int j = 1; j <= 5; j++)
            {
                minorDockspace->AddDockWindow(
					FNew(FDockWindow)
                    .AllowedDockspaces(FDockspaceFilter().WithDockTypeMask(FDockTypeMask::All))
                    .Title(String::Format("Minor {} ({})", j, i))
                    .Background(Color::RGBA(36, 36, 36))
                    .Child(
                        FNew(FLabel)
                        .Text(String::Format("This is {} minor window in {} major window", j, i))
                        .FontSize(16)
                        .HAlign(HAlign::Fill)
                        .VAlign(VAlign::Fill)
                    )
                    .Name(String::Format("Minor{}_{}", j, i))
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Fill)
                    .As<FDockWindow>()
                );
            }
        }
    }

}

