#pragma once

#include "Fusion.h"
#include "VulkanRHI.h"

#include "TestWidgets.h"

using namespace CE;

namespace WidgetTests
{
	class RendererSystem : public ApplicationMessageHandler
	{
		CE_NO_COPY(RendererSystem)
	public:

		static RendererSystem& Get()
		{
			static RendererSystem instance{};
			return instance;
		}

		void Init();
		void Shutdown();

		void Render();

		FUNCTION()
		void RebuildFrameGraph();

		void BuildFrameGraph();
		void CompileFrameGraph();

		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowDestroyed(PlatformWindow* window) override;
		void OnWindowClosed(PlatformWindow* window) override;
		void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
		void OnWindowMinimized(PlatformWindow* window) override;
		void OnWindowCreated(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;

	private:

		RendererSystem() {}

	public:

		HashMap<u64, Vec2i> windowSizesById;
		RHI::FrameScheduler* scheduler = nullptr;

		bool rebuildFrameGraph = true;
		bool recompileFrameGraph = true;
		int curImageIndex = 0;

		RHI::DrawListContext drawList{};
	};

	CLASS()
	class FusionTestWindow : public MajorDockspaceWindow
	{
		CE_CLASS(FusionTestWindow, MajorDockspaceWindow)
	public:

		FusionTestWindow();

		void Construct() override;

	private:

		FUSION_WIDGET;
	};



}

#include "FusionTest.rtti.h"
