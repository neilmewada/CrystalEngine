
#include "FusionTest.h"

namespace WidgetTests
{
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
		m_AllowSplitting = true;

		detachedDockspaceWindowClass = MajorDockspaceWindow::StaticClass();
	}

	void MajorDockspace::Construct()
	{
		Super::Construct();

		GetTabWell()->Padding(Vec4(1, 1.25f, 0, 0) * 15);

		Style("Dockspace");
	}

}