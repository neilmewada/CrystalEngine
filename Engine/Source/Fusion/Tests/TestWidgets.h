#pragma once

using namespace CE;

namespace WidgetTests
{
    CLASS()
	class MajorDockspaceWindow : public FDockspaceWindow
	{
		CE_CLASS(MajorDockspaceWindow, FDockspaceWindow)
    public:

        MajorDockspaceWindow();

        void Construct() override;

	};

    CLASS()
    class MajorDockspace : public FDockspace
    {
        CE_CLASS(MajorDockspace, FDockspace)
    public:

        MajorDockspace();

        void Construct() override;
        
    };
    
} // namespace WidgetTests

#include "TestWidgets.rtti.h"