#pragma once

#include "Fusion.h"
#include "VulkanRHI.h"

#include "TestWidgets.h"

using namespace CE;

namespace WidgetTests
{

	CLASS()
	class FusionTestWindow : public MajorDockspaceWindow
	{
		CE_CLASS(FusionTestWindow, MajorDockspaceWindow)
	public:

		FusionTestWindow();

		void Construct() override;

		void OnPaint(FPainter* painter) override;

	private:

		Ref<SceneTreeView> treeView;
		Ref<SceneTreeViewModel> treeModel;

	public:

		FUSION_PROPERTY(bool, Flipped);

		FUSION_WIDGET;
	};



}

#include "FusionTest.rtti.h"
