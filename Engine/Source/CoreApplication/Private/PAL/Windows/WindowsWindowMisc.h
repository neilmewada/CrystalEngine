#pragma once

namespace CE
{

	class WindowsWindowMisc
	{
		CE_STATIC_CLASS(WindowsWindowMisc)
	public:

		static u32 GetDpiForWindow(SDLPlatformWindow* window);

		static int GetWindowZOrder(SDLPlatformWindow* window);

		static void SetupBorderlessWindow(SDLPlatformWindow* window, bool borderless) {}

	};

	typedef WindowsWindowMisc PlatformWindowMisc;

} // namespace CE
