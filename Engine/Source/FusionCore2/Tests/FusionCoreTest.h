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

			Background(Color(0.13f, 0.13f, 0.15f));

			FGradient gradient{};
			gradient.gradientType = FGradientType::Linear;
			gradient.stops = {
				FGradientKey(0.0f, Colors::Red),
				FGradientKey(0.5f, Colors::Yellow),
				FGradientKey(1.0f, Colors::Green),
			};
			gradient.angle = Math::ToRadians(45);

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
							FGradient gradientA{};
							gradientA.gradientType = FGradientType::Linear;
							gradientA.stops = {
								FGradientKey(0.0f, Colors::Red),
								FGradientKey(0.5f, Colors::Yellow),
								FGradientKey(1.0f, Colors::Green),
							};
							gradientA.angle = Math::ToRadians(45);

							FGradient gradientB{};
							gradientB.gradientType = FGradientType::Linear;
							gradientB.stops = {
								FGradientKey(0.0f, Colors::Blue),
								FGradientKey(0.5f, Colors::Cyan),
								FGradientKey(1.0f, Colors::White),
							};
							gradientB.angle = Math::ToRadians(135);

							FBrush target = gradientToggle ? FBrush(gradientA) : FBrush(gradientB);

							FAnimate_Spring(bar3, Background)
							.Target(target)
							.Play("gradient");

							gradientToggle = !gradientToggle;
						}),

						FNew(FButton)
						.FillRatio(1.0f)
						.Height(30)
						.Name("H_2")
						.Style("Button/Secondary")
						.OnClick([this]
						{
							FAffineTransform toScale = scaleDown ? FAffineTransform::Scale(Vec2(0.5f, 0.5f)) : FAffineTransform::Identity();

							FAnimate_Spring(bar1, Transform)
							.Target(toScale)
							.Play("scale");

							scaleDown = !scaleDown;
						}),

						FNew(FButton)
						.FillRatio(1.0f)
						.Height(30)
						.Name("H_3")
						.Style("Button/Destructive")
						.OnClick([this]
						{
							FAffineTransform toRotation = rotateForward ? FAffineTransform::Rotation(Math::ToRadians(90)) : FAffineTransform::Identity();

							FAnimate_Spring(bar1, Transform)
								.Target(toRotation)
								.Play("rotate");

							rotateForward = !rotateForward;
						}),

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
					.ClipContent(true)
					.Name("Bar_2")
					.Child(
						FNew(FHorizontalStack)
						.Spacing(10)
						.ContentVAlign(VAlign::Center)
						.HAlign(HAlign::Fill)
						.VAlign(VAlign::Fill)
						.Name("hstack")
						.Transform(FAffineTransform::Rotation(Math::ToRadians(3)))
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

					FAssignNew(FDecoratedWidget, bar3)
					.Background(gradient)
					.Shape(FRectangle())
					.Height(100)
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
		Ref<FDecoratedWidget> bar3;

		bool scaleDown = true;
		bool rotateForward = true;
		bool gradientToggle = true;
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
