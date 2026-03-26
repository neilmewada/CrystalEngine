#pragma once

#include "FusionCore.h"

using namespace CE;

namespace RenderingTests
{

	CLASS()
	class TestWindow : public FDecoratedWidget
	{
		CE_CLASS(TestWindow, FDecoratedWidget)
	public:

		using SomeType = FWidget;

		String MyFunc() { return ""; }

		TestWindow()
		{

		}

		void Construct() override
		{
			Super::Construct();

			Background(FBrush(Color(0.13f, 0.13f, 0.15f)));

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
						FNew(FButton)
						.FillRatio(1.0f)
						.Height(30)
						.Name("H_1")
						.Style("Button/Primary")
						.OnClick([this]
						{
							using FuncSig = TMemberFunctionCast<TPtrType<decltype(bar1)>::Type, decltype(&TPtrType<decltype(bar1)>::Type::Transition_Transform)>::TCastedFuncSignature;
							
							auto ptr = (FuncSig)&TPtrType<decltype(bar1)>::Type::Transition_Transform;

							FAnimate::Tween(bar1, ptr)
							.From(FAffineTransform::Identity())
							.To(FAffineTransform::Scale(Vec2(0.5f, 0.5f)))
							.Duration(1.0f)
							.Easing(FEasingType::EaseInOutCubic)
							.Play(bar1, "bar1");
						}),

						FNew(FButton)
						.FillRatio(1.0f)
						.Height(30)
						.Name("H_2")
						.Style("Button/Secondary"),

						FNew(FButton)
						.FillRatio(1.0f)
						.Height(30)
						.Name("H_3")
						.Style("Button/Destructive"),

						FNew(FDecoratedWidget)
						.Background(Colors::Cyan)
						.FillRatio(1.0f)
						.Height(30)
						.Name("H_4")
						.Transform(FAffineTransform::Rotation(Math::ToRadians(5)))
					),

					FAssignNew(FDecoratedWidget, bar1)
					.Background(Colors::Cyan)
					.Height(30)
					.Name("Bar_1"),

					FAssignNew(FDecoratedWidget, bar2)
					.Background(Colors::Red)
					.Border(FPen(Colors::Green, 1.0f))
					.Shape(FRoundedRectangle(5.0f))
					.Height(30)
					.ForcePaintBoundary(true)
					.Transform(FAffineTransform::Rotation(Math::ToRadians(10)))
					.Name("Bar_2")
					.Child(
						FNew(FHorizontalStack)
						.Spacing(10)
						.ContentVAlign(VAlign::Center)
						.HAlign(HAlign::Fill)
						.VAlign(VAlign::Fill)
						.Name("hstack")
						.Padding(Vec4(25, 0, 0, 0))
						(
							FNew(FDecoratedWidget)
							.Shape(FCircle())
							.Background(Colors::Blue)
							.Width(25)
							.Height(25)
							.HAlign(HAlign::Center)
							.VAlign(VAlign::Center)
							.Name("Ball_1"),

							FNew(FButton)
							.Shape(FCircle())
							.Background(Colors::Orange)
							.Width(25)
							.Height(25)
							.HAlign(HAlign::Center)
							.VAlign(VAlign::Center)
							.Name("Ball_2")
							.OnClick([]
							{
								CE_LOG(Info, All, "Clicked Ball_2");
							})
						)
					),

					FNew(FDecoratedWidget)
					.Background(Colors::Yellow)
					.Shape(FRectangle())
					.Height(30)
					.Name("Bar_3"),

					FNew(FWidget)
					.FillRatio(1.0f)
					.Name("Whitespace")
				)
			);
		}


		Ref<FVerticalStack> vstack;
		Ref<FHorizontalStack> hstack;
		Ref<FDecoratedWidget> bar1;
		Ref<FDecoratedWidget> bar2;
	};

	CLASS()
	class FusionRenderService : public FRenderService, public ApplicationMessageHandler
	{
		CE_CLASS(FusionRenderService, FRenderService)
	public:

		void OnStart() override;

		void OnShutdown() override;

		void MarkFrameGraphDirty() override;

		void RenderPrepare() override;

		bool BeginRender() override;

		void EndRender() override;

		void SetScopeDrawPackets(FSurface* surface);

		int GetCurrentFrameIndex() override { return curImageIndex; }

		void BuildFrameGraph();
		void CompileFrameGraph();

		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowDestroyed(PlatformWindow* window) override;
		void OnWindowClosed(PlatformWindow* window) override;
		void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
		void OnWindowMinimized(PlatformWindow* window) override;
		void OnWindowCreated(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;
		void OnWindowShown(PlatformWindow* window) override;

		bool rebuildFrameGraph = true;
		bool recompileFrameGraph = true;
		int curImageIndex = 0;
		int frameGraphBuildSubmissionIndex = -1;

		RHI::DrawListContext drawList{};
	};

}

#include "FusionCoreTest.rtti.h"
