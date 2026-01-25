#pragma once

namespace CE
{

    class LinuxWindowMisc
    {
        CE_STATIC_CLASS(LinuxWindowMisc)
    public:

        static u32 GetDpiForWindow(SDLPlatformWindow* sdlWindow);

        static void SetupBorderlessWindow(SDLPlatformWindow* window, bool borderless) {}

        static int GetWindowZOrder(SDLPlatformWindow* window);

    };

    typedef LinuxWindowMisc PlatformWindowMisc;

} // namespace CE
