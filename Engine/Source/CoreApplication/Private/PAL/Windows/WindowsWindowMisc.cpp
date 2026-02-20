
#include "CoreApplication.h"

#include "PAL/Common/PlatformWindowMisc.h"

#include <SDL3/SDL.h>

#include <Windows.h>
#include <shellscalingapi.h>

namespace CE
{
    static HWND GetHWND(SDL_Window* sdlWindow)
	{
		return (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    }

    u32 WindowsWindowMisc::GetDpiForWindow(SDLPlatformWindow* window)
    {
        if (window == nullptr)
            return GetDpiForSystem();

        HWND hWnd = (HWND)window->GetOSNativeHandle();
        return ::GetDpiForWindow(hWnd);
    }

    int WindowsWindowMisc::GetWindowZOrder(SDLPlatformWindow* window)
    {
        HWND target = GetHWND(window->GetSdlHandle());
        if (!target)
            return -1;

        int z = 0;

        for (HWND h = GetTopWindow(nullptr); h != nullptr; h = GetNextWindow(h, GW_HWNDNEXT)) 
        {
            if (h == target)
                return z;
            ++z;
        }

        return -1;
    }
} // namespace CE
