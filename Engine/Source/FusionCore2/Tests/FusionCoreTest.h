#pragma once

#include "FusionCore.h"

using namespace CE;

namespace RenderingTests
{
	CLASS()
	class CustomWidget : public FDecoratedWidget
	{
		CE_CLASS(CustomWidget, FDecoratedWidget)
	public:

		void Construct() override
		{
			Super::Construct();


		}

	};

	CLASS()
	class TestWindow : public FDecoratedWidget
	{
		CE_CLASS(TestWindow, FDecoratedWidget)
	public:

		void Construct() override
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

			FBrush hsvColorMap("res:/Images/HSVColorMap.png");
			hsvColorMap.SetImageFit(FImageFit::Contain);
			hsvColorMap.SetBrushTiling(FBrushTiling::TileXY);

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
							FAffineTransform toScale = scaleDown ? FAffineTransform::Scale(Vec2(0.5f, 0.5f)) : FAffineTransform::Identity();
							
							FAnimate_Spring(bar1, Transform)
							.Target(toScale)
							.Play();

							colorIdx = (colorIdx + 1) % COUNTOF(colors);

							//bar4->Background(colors[colorIdx]);

							FAffineTransform toRotation = colorIdx % 2 != 0 ? FAffineTransform::Rotation(Math::ToRadians(90)) : FAffineTransform::Identity();

							//bar4->Transform(toRotation);

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
					.Background(Colors::White)
					.Border(FPen(Colors::Red, 2.0f))
					.Shape(FRoundedRectangle(5.0f))
					.Height(30)
					//.ForcePaintBoundary(true)
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
					.Shape(FRoundedRectangle(8.0f))
					.Height(120)
					.Name("Bar_3"),

					FAssignNew(CustomWidget, bar4)
					.Background(hsvColorMap)
					.Height(120),

					FNew(FWidget)
					.FillRatio(1.0f)
					.Name("Whitespace")
				)
			);
		}

		TestWindow()
		{
			m_GradientOffset = 0.0f;
			m_DashLength = 10.0f;
			m_DashGap = 5.0f;
			m_DashPhase = 0.0f;
		}

		void PaintOverlay(FPainter& painter) override
		{
			Super::PaintOverlay(painter);

			const Vec2 sz = GetLayoutSize();

			// Mirrored gradient: blue → cyan → green → yellow → green → cyan → blue
			FGradient grad{};
			grad.gradientType = FGradientType::Linear;
			grad.stops = {
				FGradientKey(0.00f, Color(0.10f, 0.20f, 1.00f)), // blue
				FGradientKey(0.17f, Color(0.00f, 0.85f, 0.90f)), // cyan
				FGradientKey(0.33f, Color(0.10f, 0.90f, 0.30f)), // green
				FGradientKey(0.50f, Color(0.95f, 0.95f, 0.10f)), // yellow
				FGradientKey(0.67f, Color(0.10f, 0.90f, 0.30f)), // green
				FGradientKey(0.83f, Color(0.00f, 0.85f, 0.90f)), // cyan
				FGradientKey(1.00f, Color(0.10f, 0.20f, 1.00f)), // blue
			};

			FPen pen(grad, 4.0f);
			pen.SetGradientOffset(m_GradientOffset);
			pen.SetStyle(FPenStyle::Dashed);
			pen.SetDashLength(m_DashLength);
			pen.SetDashGap(m_DashGap);
			pen.SetDashPhase(m_DashPhase);
			painter.SetPen(pen);
			painter.SetBrush(FBrush());

			// Drawing area: horizontally centered, in the lower half
			const f32 drawW = Math::Min(sz.x * 0.80f, 500.0f);
			const f32 drawH = drawW * 0.35f;
			const Vec2 orig = Vec2((sz.x - drawW) * 0.5f, sz.y * 0.62f);

			auto P = [&](f32 x, f32 y) -> Vec2
			{
				return Vec2(orig.x + x * drawW, orig.y + y * drawH);
			};

			// Simple debug path: straight line → cubic curve → straight line → cubic curve
			// Easy to reason about: gradient should flow left-to-right along the whole path
			painter.PathClear();

			// Segment 1: straight horizontal line from left
			painter.PathInsert(P(0.00f, 0.50f));
			painter.PathInsert(P(0.20f, 0.50f));

			// Segment 2: cubic curve swooping up then down
			painter.PathBezierCubicCurveTo(
				P(0.20f, 0.50f), P(0.30f, 0.00f), P(0.45f, 0.00f), P(0.50f, 0.50f));

			// Segment 3: straight line through the middle
			painter.PathInsert(P(0.65f, 0.50f));

			// Segment 4: cubic curve swooping down then up to the right end
			painter.PathBezierCubicCurveTo(
				P(0.65f, 0.50f), P(0.75f, 1.00f), P(0.88f, 1.00f), P(1.00f, 0.50f));

			painter.PathStroke(false, true);
		}

		FUSION_PROPERTY(f32, GradientOffset);
		FUSION_PROPERTY(f32, DashLength);
		FUSION_PROPERTY(f32, DashGap);
		FUSION_PROPERTY(f32, DashPhase);

		Ref<FVerticalStack>   vstack;
		Ref<FHorizontalStack> hstack;
		Ref<FDecoratedWidget> bar1;
		Ref<FDecoratedWidget> bar2;
		Ref<FDecoratedWidget> bar3;
		Ref<CustomWidget>	  bar4;

		int  colorIdx = 0;
		bool scaleDown = true;
		bool rotateForward = true;
		bool gradientToggle = false;
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
