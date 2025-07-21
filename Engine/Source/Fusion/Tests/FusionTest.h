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

	private:

		FUSION_WIDGET;
	};



}

#include "FusionTest.rtti.h"
