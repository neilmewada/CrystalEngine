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


    CLASS()
    class MinorDockspaceWindow : public FDockspaceWindow
    {
        CE_CLASS(MinorDockspaceWindow, FDockspaceWindow)
    public:

        MinorDockspaceWindow();

        void Construct() override;

    };

    CLASS()
    class MinorDockspace : public FDockspace
    {
        CE_CLASS(MinorDockspace, FDockspace)
    public:

        MinorDockspace();

        void Construct() override;

    };
    
} // namespace WidgetTests

#include "TestWidgets.rtti.h"