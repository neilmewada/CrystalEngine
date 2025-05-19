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

    void RenderingTestWidget::Construct()
    {
        Super::Construct();

        FBrush transparentPattern = FBrush("/Engine/Resources/Icons/TransparentPattern", Color::White());
        transparentPattern.SetBrushTiling(FBrushTiling::TileXY);
        transparentPattern.SetBrushSize(Vec2(16, 16));

        FButton* openPopupBtn = nullptr;
        FTextButton* nativePopupBtn = nullptr;

        PlatformApplication::Get()->AddMessageHandler(this);

        treeViewModel = CreateObject<TreeViewModel>(this, "TreeViewModel");
        listViewModel = CreateObject<ListViewModel>(this, "ListViewModel");

        FGradient gradient = FGradient();
        gradient.stops = {
            FGradientKey(0.00f, Color::Red()),
            FGradientKey(0.25f, Color::Yellow()),
            FGradientKey(0.50f, Color::Green()),
            FGradientKey(0.75f, Color::Cyan()),
            FGradientKey(1.00f, Color::Blue()),
        };

        FBrush gradientBrush = FBrush(gradient);

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
                    .Padding(Vec4(1, 1, 1, 1) * 20)
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
                            .Background(Color::Yellow())
                            .Height(25)
                            .FillRatio(0.25f),

                            FNew(FStyledWidget)
                            .Background(Color::Green())
                            .Height(25)
                            .FillRatio(0.5f),

                            FNew(FStyledWidget)
                            .Background(Color::Cyan())
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
                        .Angle(0)
                        (
                            FAssignNew(FLabel, buttonLabel)
                            .FontSize(10)
                            .Text("Click Count 0")
                        ),

                        FNew(FListView)
                        .GenerateRowCallback(MemberDelegate(&Self::GenerateListViewRow, this))
                        .RowHeight(50)
                        .Model(listViewModel)
                        .Height(120)
                        .HAlign(HAlign::Fill)

                        /*FNew(FTreeView)
                        .GenerateRowDelegate(MemberDelegate(&Self::GenerateTreeViewRow, this))
                        .Model(treeViewModel)
                        .RowHeight(25)
                        .Header(
		                    FNew(FTreeViewHeader)
		                    .Columns(
		                        FNew(FTreeViewHeaderColumn)
		                        .Title("Name")
		                        .FillRatio(0.8f),

		                        FNew(FTreeViewHeaderColumn)
		                        .Title("Type")
		                        .FillRatio(0.2f)
		                    )
		                )
                        .Height(120)
                        .HAlign(HAlign::Fill)*/
                    )
                )
            )
        );

        //windowContent->Enabled(false);
    }

    FTreeViewRow& RenderingTestWidget::GenerateTreeViewRow()
    {
        FTreeViewRow& row = FNew(FTreeViewRow);

        row.Cells(
            FNew(FTreeViewCell)
            .Text("Name")
            .ArrowEnabled(true)
            .FontSize(10),

            FNew(FTreeViewCell)
            .Text("Type")
            .FontSize(10)
            .Foreground(Color::RGBA(255, 255, 255, 140))
            .ArrowEnabled(false)
        );

        return row;
    }

    FListViewRow& RenderingTestWidget::GenerateListViewRow()
    {
        return
        FNew(FListViewRow)
        .Child(
            FNew(FVerticalStack)
            .Gap(2.5f)
            .ContentHAlign(HAlign::Left)
            .VAlign(VAlign::Center)
            .HAlign(HAlign::Left)
            .Padding(Vec4(10, 0, 10, 0))
            (
                FNew(FLabel)
                .Text("Title"),

                FNew(FLabel)
                .FontSize(8)
                .Text("This is description!")
                .Foreground(Color::RGBA(255, 255, 255, 140))
            )
        )
        .As<FListViewRow>();
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

