
#include "CoreApplication.h"

#include "PAL/Common/PlatformWindowMisc.h"

#include <SDL2/SDL.h>
#include <SDL_syswm.h>

#include <Windows.h>
#include <shellscalingapi.h>

namespace CE
{
    static HWND GetHWND(SDL_Window* sdlWindow)
	{
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);

        if (SDL_GetWindowWMInfo(sdlWindow, &wmInfo)) 
        {
            return wmInfo.info.win.window;
        }

        return nullptr;
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
