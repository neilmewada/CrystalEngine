#include "FusionCoreTest.h"


namespace RenderingTests
{


#pragma region Renderer System

	void RendererSystem::Init()
	{
        PlatformApplication::Get()->AddMessageHandler(this);
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

        auto scheduler = RHI::FrameScheduler::Get();

        int imageIndex = scheduler->BeginExecution();

        if (imageIndex >= RHI::Limits::MaxSwapChainImageCount || rebuildFrameGraph || recompileFrameGraph)
        {
            rebuildFrameGraph = recompileFrameGraph = true;
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
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::OnWindowDestroyed(PlatformWindow* window)
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::OnWindowClosed(PlatformWindow* window)
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::OnWindowMinimized(PlatformWindow* window)
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::OnWindowCreated(PlatformWindow* window)
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

    void RendererSystem::OnWindowExposed(PlatformWindow* window)
    {
        rebuildFrameGraph = recompileFrameGraph = true;
    }

#pragma endregion

	void RenderingTestWidget::OnTimeOut()
	{
        gradientAngle += 0.1f;

        FGradient gradient = FGradient();
        gradient.angle = 90;
        gradient.stops = {
            FGradientKey(0.00f, Colors::Red),
            FGradientKey(0.25f, Colors::Yellow),
            FGradientKey(0.50f, Colors::Green),
            FGradientKey(0.75f, Colors::Cyan),
            FGradientKey(1.00f, Colors::Blue),
        };
	}

    void RenderingTestWidget::Construct()
    {
        Super::Construct();

        timer = CreateObject<FTimer>(this, "Timer");
        timer->OnTimeOut(FUNCTION_BINDING(this, OnTimeOut));
        timer->Start(16);

        FBrush transparentPattern = FBrush("/Engine/Resources/Icons/TransparentPattern", Colors::White);
        transparentPattern.SetBrushTiling(FBrushTiling::TileXY);
        transparentPattern.SetBrushSize(Vec2(16, 16));

        PlatformApplication::Get()->AddMessageHandler(this);

        FGradient gradient = FGradient();
        gradient.angle = 0;
        gradient.stops = {
            FGradientKey(0.00f, Colors::Red),
            FGradientKey(0.25f, Colors::Yellow),
            FGradientKey(0.50f, Colors::Green),
            FGradientKey(0.75f, Colors::Cyan),
            FGradientKey(1.00f, Colors::Blue),
        };

        FBrush gradientBrush = FBrush(gradient);
        gradientBrush.SetBrushTiling(FBrushTiling::TileXY);

        Child(
            FAssignNew(FStyledWidget, borderWidget)
            .Background(FBrush(Color::RGBA(36, 36, 36)))
            .BorderWidth(1.0f)
            .BorderColor(Color::RGBA(15, 15, 15))
            .Padding(Vec4(1, 1, 1, 1))
            .Name("RootStyle")
            (
                FAssignNew(FVerticalStack, rootBox)
                .ContentHAlign(HAlign::Fill)
                .Name("RootBox")
                (
                    FNew(FTitleBar)
                    .Background(Color::RGBA(26, 26, 26))
                    .Height(40)
                    .HAlign(HAlign::Fill)
                    (
                        FNew(FOverlayStack)
                        .VAlign(VAlign::Fill)
                        .HAlign(HAlign::Fill)
                        (
                            FNew(FHorizontalStack)
                            .ContentVAlign(VAlign::Center)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            .Name("TitleLabelStack")
                            (
                                FNew(FWidget)
                                .FillRatio(1.0f),

                                FNew(FLabel)
                                .FontSize(12)
                                .Text("Window Title")
                                .HAlign(HAlign::Center)
                                .VAlign(VAlign::Center)
                                .Name("TitleLabel")
                                .Angle(15),

                                FNew(FWidget)
                                .FillRatio(1.0f)
                            ),

                            FNew(FHorizontalStack)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            .Name("ControlStack")
                            (
                                FNew(FWidget)
                                .FillRatio(1.0f),

                                FNew(FButton)
                                .OnClicked([this]
                                    {
                                        static_cast<FNativeContext*>(GetContext())->Minimize();
                                    })
                                .Padding(Vec4(17, 8, 17, 8))
                                .Name("WindowMinimizeButton")
                                .Style("Button.WindowControl")
                                .VAlign(VAlign::Top)
                                (
                                    FNew(FImage)
                                    .Background(FBrush("/Engine/Resources/Icons/MinimizeIcon"))
                                    .Width(11)
                                    .Height(11)
                                    .HAlign(HAlign::Center)
                                    .VAlign(VAlign::Center)
                                ),

                                FNew(FButton)
                                .OnClicked([this]
                                    {
                                        FNativeContext* nativeContext = static_cast<FNativeContext*>(GetContext());
                                        if (nativeContext->IsMaximized())
                                        {
                                            nativeContext->Restore();
                                        }
                                        else
                                        {
                                            nativeContext->Maximize();
                                        }
                                    })
                                .Padding(Vec4(17, 8, 17, 8))
                                .Name("WindowMaximizeButton")
                                .Style("Button.WindowControl")
                                .VAlign(VAlign::Top)
                                (
                                    FAssignNew(FImage, maximizeIcon)
                                    .Background(FBrush("/Engine/Resources/Icons/MaximizeIcon"))
                                    .Width(11)
                                    .Height(11)
                                    .HAlign(HAlign::Center)
                                    .VAlign(VAlign::Center)
                                ),

                                FNew(FButton)
                                .OnClicked([this]
                                    {
                                        RequestEngineExit("USER_QUIT");
                                    })
                                .Padding(Vec4(18, 8, 18, 8))
                                .Name("WindowCloseButton")
                                .Style("Button.WindowClose")
                                .VAlign(VAlign::Top)
                                (
                                    FNew(FImage)
                                    .Background(FBrush("/Engine/Resources/Icons/CrossIcon"))
                                    .Width(10)
                                    .Height(10)
                                    .HAlign(HAlign::Center)
                                    .VAlign(VAlign::Center)
                                    .Name("CloseBtn")
                                )
                            )
                        )
                    ),

                    // Window Content Begins

                    FAssignNew(FVerticalStack, windowContent)
                    .Padding(Vec4(10, 10, 10, 10))
                    .Name("ContentVStack")
                    .FillRatio(1.0f)
                    (
						FNew(FHorizontalStack)
                        .ContentVAlign(VAlign::Fill)
                        .Name("HStack1")
                        (
                            FNew(FStyledWidget)
                            .Background(transparentPattern)
                            .BackgroundShape(FRoundedRectangle(2.5f, 5, 7.5f, 10))
                            .FillRatio(1.0f)
                            .MinWidth(60)
                            .MinHeight(30)
                        )
                        .Margin(Vec4(0, 0, 0, 5)),

                        FAssignNew(FSplitBox, splitBox)
                        .Direction(FSplitDirection::Horizontal)
                        .Angle(0)
                        (
							FNew(FStyledWidget)
                            .Background(Colors::Yellow)
                            .Height(25)
                            .FillRatio(0.25f),

                            FNew(FStyledWidget)
                            .Background(Colors::Green)
                            .Height(25)
                            .FillRatio(0.5f),

                            FNew(FStyledWidget)
                            .Background(Colors::Cyan)
                            .Height(25)
                            .FillRatio(0.25f)
                        ),

                        FAssignNew(FButton, button)
                        .OnClicked([this]
                            {
                                buttonLabel->Text(String::Format("Click Count {}", ++hitCounter));
                                splitBox->Angle(hitCounter % 2 == 0 ? 15 : 0);
                            })
                        .Name("Button")
                        .Scale(Vec2(0.75f, 0.75f))
                        .Angle(15)
                        (
                            FAssignNew(FLabel, buttonLabel)
                            .FontSize(10)
                            .Text("Click Count 0")
                        ),

                        FNew(FStyledWidget)
                        .Background(gradientBrush)
                        .Height(40)
                    )
                )
            )
        );

        //windowContent->Enabled(false);
    }

    void RenderingTestWidget::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

    }

    void RenderingTestWidget::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        PlatformApplication::Get()->RemoveMessageHandler(this);
    }

    void RenderingTestWidget::OnWindowRestored(PlatformWindow* window)
    {
        FNativeContext* nativeContext = static_cast<FNativeContext*>(GetContext());

        if (nativeContext->GetPlatformWindow() == window)
        {
            //maximizeIcon->Background(FBrush("/Engine/Resources/Icons/MaximizeIcon"));
            this->Padding(Vec4());
        }
    }

    void RenderingTestWidget::OnWindowMaximized(PlatformWindow* window)
    {
        FNativeContext* nativeContext = static_cast<FNativeContext*>(GetContext());

        if (nativeContext->GetPlatformWindow() == window)
        {
            //maximizeIcon->Background(FBrush("/Engine/Resources/Icons/RestoreIcon"));
            this->Padding(Vec4(1, 1, 1, 1) * 4.0f);
        }
    }

    void RenderingTestWidget::OnWindowExposed(PlatformWindow* window)
    {
        FNativeContext* nativeContext = static_cast<FNativeContext*>(GetContext());

        if (nativeContext->GetPlatformWindow() == window && !window->IsMaximized())
        {
            OnWindowRestored(window);
        }
    }

}
