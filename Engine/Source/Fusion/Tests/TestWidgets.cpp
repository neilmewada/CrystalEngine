
#include "FusionTest.h"

namespace WidgetTests
{
	// ------------------------------------------------
	// - Major Dockspace

	MajorDockspaceWindow::MajorDockspaceWindow()
	{
		m_DockspaceClass = MajorDockspace::StaticClass();
	}

	void MajorDockspaceWindow::Construct()
	{
		Super::Construct();

	}

	MajorDockspace::MajorDockspace()
	{
		m_DockspaceType = FDockTypeMask::Major;
		m_AllowDocking = true;
		m_AllowSplitting = false;

		detachedDockspaceWindowClass = MajorDockspaceWindow::StaticClass();
	}

	void MajorDockspace::Construct()
	{
		Super::Construct();

		container->GetTabWell()->Padding(Vec4(1, 1.25f, 0, 0) * 15);

		Style("Dockspace");
	}

	// ------------------------------------------------
	// - Minor Dockspace

	MinorDockspaceWindow::MinorDockspaceWindow()
	{
		m_DockspaceClass = MinorDockspace::StaticClass();
	}

	void MinorDockspaceWindow::Construct()
	{
		Super::Construct();

		
	}

	MinorDockspace::MinorDockspace()
	{
		m_DockspaceType = FDockTypeMask::Minor;
		m_AllowDocking = true;
		m_AllowSplitting = true;

		detachedDockspaceWindowClass = MinorDockspaceWindow::StaticClass();
	}

	void MinorDockspace::Construct()
	{
		Super::Construct();

		Style("MinorDockspace");
	}
}
