#include "FusionCoreTest.h"


namespace RenderingTests
{
    void TestWindow::Construct()
    {
        Super::Construct();

        Background(Color(0.13f, 0.13f, 0.15f));

        constexpr Color colors[] = {
            Colors::Red, Colors::Orange, Colors::Yellow, Colors::Green, Colors::Cyan, Colors::Purple, Colors::Blue
        };

        FGradient gradient{};
        gradient.gradientType = FGradientType::Linear;
        gradient.stops = {
            FGradientKey(0.0f,  Color(0.80f, 0.00f, 0.10f)),  // deep red
            FGradientKey(0.25f, Color(1.00f, 0.35f, 0.00f)),  // orange
            FGradientKey(0.65f, Color(1.00f, 0.88f, 0.10f)),  // yellow
            FGradientKey(1.0f,  Color(1.00f, 1.00f, 0.88f)),  // warm white
        };
        gradient.angle = Math::ToRadians(30);

        FGradient penGrad{};
        penGrad.gradientType = FGradientType::Linear;
        penGrad.stops = {
            FGradientKey(0.00f, Color(0.10f, 0.00f, 0.90f)), // electric indigo
            FGradientKey(0.25f, Color(0.70f, 0.00f, 1.00f)), // violet
            FGradientKey(0.50f, Color(0.00f, 0.90f, 1.00f)), // cyan
            FGradientKey(0.75f, Color(0.70f, 0.00f, 1.00f)), // violet
            FGradientKey(1.00f, Color(0.10f, 0.00f, 0.90f)), // electric indigo
        };


        FBrush imageBrush("res:/Icons/TransparentPattern.png");
        imageBrush.SetImageFit(FImageFit::Fill);
        imageBrush.SetBrushTiling(FBrushTiling::TileXY);
        imageBrush.SetBrushSize(Vec2(16, 16));

        Child(
            FAssignNew(FVerticalStack, vstack)
            .ContentHAlign(HAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .Enabled(true)
            .Visible(true)
            .Padding(FMargin(5, 5, 5, 5))
            .Spacing(10)
            .Name("RootStack")
            (
                FNew(FWidget)
                .Height(25)
                .Name("Empty"),

                FAssignNew(FHorizontalStack, hstack)
                .ContentHAlign(HAlign::Center)
                .ContentVAlign(VAlign::Center)
                .ClipContent(true)
                .ForcePaintBoundary(true)
                .Spacing(10)
                .Name("hstack")
                (
                    FAssignNew(FButton, btn1)
                    .FillRatio(1.0f)
                    .Height(30)
                    .Name("H_1")
                    .Style("Button/Primary")
                    .OnClick([this]
                    {
                        // Sunset fire: deep red → orange → yellow → warm white, shallow angle
                        FGradient gradientA{};
                        gradientA.gradientType = FGradientType::Linear;
                        gradientA.stops = {
                            FGradientKey(0.0f,  Color(0.80f, 0.00f, 0.10f)),
                            FGradientKey(0.25f, Color(1.00f, 0.35f, 0.00f)),
                            FGradientKey(0.65f, Color(1.00f, 0.88f, 0.10f)),
                            FGradientKey(1.0f,  Color(1.00f, 1.00f, 0.88f)),
                        };
                        gradientA.angle = Math::ToRadians(30);

                        // Aurora: deep indigo → violet → cyan → mint, steep angle
                        FGradient gradientB{};
                        gradientB.gradientType = FGradientType::Linear;
                        gradientB.stops = {
                            FGradientKey(0.0f,  Color(0.05f, 0.00f, 0.40f)),
                            FGradientKey(0.35f, Color(0.55f, 0.00f, 0.85f)),
                            FGradientKey(0.70f, Color(0.00f, 0.80f, 0.90f)),
                            FGradientKey(1.0f,  Color(0.20f, 1.00f, 0.65f)),
                        };
                        gradientB.angle = Math::ToRadians(150);

                        FBrush target = gradientToggle ? FBrush(gradientA) : FBrush(gradientB);

                        FAnimate_Spring(bar3, Background)
                        .Target(target)
                        .Play();

                        gradientToggle = !gradientToggle;
                    }),

                    FNew(FButton)
                    .FillRatio(1.0f)
                    .Height(30)
                    .Name("H_2")
                    .Style("Button/Primary")
                    .OnClick([this]
                    {
                        // Neon aurora border: indigo → violet → cyan  ↔  magenta → orange → gold
                        FGradient penGradA{};
                        penGradA.gradientType = FGradientType::Linear;
                        penGradA.stops = {
                            FGradientKey(0.00f, Color(0.10f, 0.00f, 0.90f)), // electric indigo
                            FGradientKey(0.25f, Color(0.70f, 0.00f, 1.00f)), // violet
                            FGradientKey(0.50f, Color(0.00f, 0.90f, 1.00f)), // cyan
                            FGradientKey(0.75f, Color(0.70f, 0.00f, 1.00f)), // violet
                            FGradientKey(1.00f, Color(0.10f, 0.00f, 0.90f)), // electric indigo
                        };

                        FGradient penGradB{};
                        penGradB.gradientType = FGradientType::Linear;
                        penGradB.stops = {
                            FGradientKey(0.000f, Color(1.00f, 0.00f, 0.50f)), // magenta
                            FGradientKey(0.125f, Color(1.00f, 0.00f, 0.00f)), // red
                            FGradientKey(0.250f, Color(1.00f, 0.50f, 0.00f)), // orange
                            FGradientKey(0.375f, Color(1.00f, 1.00f, 0.00f)), // yellow
                            FGradientKey(0.500f, Color(0.00f, 1.00f, 0.00f)), // green
                            FGradientKey(0.625f, Color(0.00f, 1.00f, 1.00f)), // cyan
                            FGradientKey(0.750f, Color(0.00f, 0.00f, 1.00f)), // blue
                            FGradientKey(0.875f, Color(0.50f, 0.00f, 1.00f)), // violet
                            FGradientKey(1.000f, Color(1.00f, 0.00f, 0.50f)), // magenta
                        };

                        FAnimate_Tween(bar2, Border)
                        .From(FPen(penGradB, 2.5f))
                        .To(FPen(penGradB, 2.5f).GradientOffset(1.0f))
                        .Duration(7.0f)
                        .Easing(FEasingType::Linear)
                        .Loop(FAnimationLoopMode::Loop)
                        .Play();

                        FAnimate_Tween(this, GradientOffset)
                        .From(1.0f)
                        .To(0.0f)
                        .Duration(5.0f)
                        .Easing(FEasingType::Linear)
                        .Loop(FAnimationLoopMode::Loop)
                        .Play();

                        FAnimate_Tween(this, DashPhase)
                        .From(0.0f)
                        .To(15.0f)
                        .Duration(1.5f)
                        .Easing(FEasingType::Linear)
                        .Loop(FAnimationLoopMode::Loop)
                        .Play();
                    }),

                    FNew(FButton)
                    .FillRatio(1.0f)
                    .Height(30)
                    .Name("H_3")
                    .Style("Button/Secondary")
                    .OnClick([this, colors]
                    {
                        FAffineTransform toScale = scaleDown ? FAffineTransform::Rotation(Math::ToRadians(90)) : FAffineTransform::Identity();
                        
                        FAnimate_Spring(bar1, Transform)
                        .Target(toScale)
                        .Play();

                        colorIdx = (colorIdx + 1) % COUNTOF(colors);

                        FBrush imageBrush("res:/Icons/TransparentPattern.png");
                        imageBrush.SetImageFit(FImageFit::Fill);
                        imageBrush.SetBrushTiling(FBrushTiling::TileXY);
                        imageBrush.SetBrushSize(scaleDown ? Vec2(32, 32) * 1.5f : Vec2(16, 16));

                        bar4->Background(imageBrush); // Automatic transition animation applied for "registered" properties

                        FAffineTransform toRotation = colorIdx % 2 != 0 ? FAffineTransform::Rotation(Math::ToRadians(90)) : FAffineTransform::Identity();

                        scaleDown = !scaleDown;
                    }),

                    FNew(FButton)
                    .FillRatio(1.0f)
                    .Height(30)
                    .Name("H_4")
                    .Style("Button/Destructive")
                    .OnClick([this]
                    {
                        FAnimate_Sequence(this, "bar1_demo")
                        .Then(FAnimate_Tween(bar1, Transform)
                            .To(FAffineTransform::Scale(Vec2(0.5f, 0.5f)))
                            .Duration(0.5f)
                        )
                        .Then(FAnimate_Tween(bar1, Transform)
                            .From(FAffineTransform::Scale(Vec2(0.5f, 0.5f)))
                            .To(FAffineTransform::Scale(Vec2(1, 1)))
                            .Duration(0.5f)
                        )
                        .Play();

                    }),

                    FNew(FButton)
                    .FillRatio(1.0f)
                    .Height(30)
                    .Name("H_5")
                    .Style("Button/Secondary")
                    .OnClick([this]
                    {
                        
                    })
                ),

                FAssignNew(FDecoratedWidget, bar1)
                .Background(Colors::Cyan)
                .Height(30)
                .Name("Bar_1"),

                FAssignNew(FDecoratedWidget, bar2)
                .Background(Colors::White)
                .Border(FPen(Colors::Red, 2.0f))
                .Shape(FRoundedRectangle(5.0f))
                .Height(30)
                //.ForcePaintBoundary(true)
                .ClipContent(true)
                .Name("Bar_2"),

                FAssignNew(FDecoratedWidget, bar3)
                .Background(gradient)
                .Shape(FRoundedRectangle(8.0f))
                .Height(120)
                .Name("Bar_3"),

                FAssignNew(CustomWidget, bar4)
                .Background(imageBrush)
                .Shape(FRoundedRectangle(5.0f))
                .Height(120),

                FNew(FWidget)
                .FillRatio(1.0f)
                .Name("Whitespace")
            )
        );
    }

#pragma region Render Service

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
        ZoneScoped;

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
        ZoneScoped;

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
        ZoneScoped;

        auto scheduler = RHI::FrameScheduler::Get();

        auto application = this->application.Lock();
        if (!application)
            return;

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
                surface->FlushDrawPackets(drawList, curImageIndex);
            }
        }

        for (int i = 0; i < application->GetSurfaceCount(); i++)
        {
            if (Ref<FSurface> surface = application->GetSurface(i))
            {
                SetScopeDrawPackets(surface.Get());
            }
        }

        drawList.Finalize();

        scheduler->EndExecution();
	}

	void FusionRenderService::SetScopeDrawPackets(FSurface* surface)
	{
        if (!surface)
            return;

        ZoneScoped;

        auto scheduler = RHI::FrameScheduler::Get();

        RHI::DrawList& surfaceDrawList = drawList.GetDrawListForTag(surface->GetDrawListTag());

        scheduler->SetScopeDrawList(surface->GetScopeId(), &surfaceDrawList);

		for (int i = 0; i < surface->GetChildSurfaceCount(); ++i)
		{
            SetScopeDrawPackets(surface->GetChildSurface(i).Get());
		}
	}

	void FusionRenderService::BuildFrameGraph()
	{
        ZoneScoped;

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

#pragma endregion

}

