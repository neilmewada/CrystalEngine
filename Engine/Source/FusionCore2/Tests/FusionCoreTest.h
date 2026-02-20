#pragma once

#include "FusionCore.h"

using namespace CE;

namespace RenderingTests
{
	CLASS()
	class TestWindow : public FCompoundWidget
	{
		CE_CLASS(TestWindow, FCompoundWidget)
	public:

		TestWindow()
		{
			
		}

		void Construct() override
		{
			Super::Construct();

			Child(
				FAssignNew(FVerticalStack, vstack)
				.HAlign(HAlign::Fill)
				.VAlign(VAlign::Fill)
				.Enabled(true)
				.Visible(true)
				.Name("RootStack")
				(
					FNew(FWidget)
					.Height(25),

					FNew(FHorizontalStack, hstack)
					.ContentHAlign(HAlign::Center)
					.ContentVAlign(VAlign::Center)
					(
						FNew(FWidget)
						.FillRatio(1.0f)
						.Height(25)
						.Name("H_1"),

						FNew(FWidget)
						.FillRatio(2.0f)
						.Height(30)
						.Name("H_2"),

						FNew(FWidget)
						.FillRatio(1.0f)
						.Height(50)
						.Name("H_3")
					),

					FNew(FWidget)
					.FillRatio(1.0f)
				)
			);
		}

	
		Ref<FVerticalStack> vstack;
		Ref<FHorizontalStack> hstack;
	};

	CLASS()
	class FusionRendererService : public FRenderService, public ApplicationMessageHandler
	{
		CE_CLASS(FusionRendererService, FRenderService)
	public:

		void OnStart() override;

		void OnShutdown() override;

		void MarkFrameGraphDirty() override;

		void RenderPrepare() override;

		void Render() override;

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
