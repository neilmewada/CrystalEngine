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

        void Construct() override;

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

		Ref<FButton>		  btn1;
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
