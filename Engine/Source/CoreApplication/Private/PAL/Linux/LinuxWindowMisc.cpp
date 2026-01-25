#include "CoreApplication.h"

#include "PAL/Common/PlatformWindowMisc.h"

#include <SDL2/SDL.h>
#include <SDL_syswm.h>

#include <X11/Xlib.h>

namespace CE
{

    u32 LinuxWindowMisc::GetDpiForWindow(SDLPlatformWindow* sdlWindow)
    {
        if (sdlWindow == nullptr)
            return PlatformApplication::Get()->GetSystemDpi();

        SDL_Window* handle = (SDL_Window*)sdlWindow->GetUnderlyingHandle();

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_bool success = SDL_GetWindowWMInfo(handle, &wmInfo);
        if (success == SDL_FALSE)
        {
            return PlatformApplication::Get()->GetSystemDpi();
        }

        double xres, yres;
        Display *dpy = wmInfo.info.x11.display;

        Window window = (Window)sdlWindow->GetOSNativeHandle();
        XWindowAttributes attribs = {};

        XGetWindowAttributes(dpy, window, &attribs);
        Screen* screen = attribs.screen;

        /*
         * there are 2.54 centimeters to an inch; so there are 25.4 millimeters.
         *
         *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
         *         = N pixels / (M inch / 25.4)
         *         = N * 25.4 pixels / M inch
         */
        xres = ((((double) screen->width) * 25.4) /
            ((double) screen->mwidth));
        yres = ((((double) screen->height) * 25.4) /
            ((double) screen->mheight));

        const int x = (int)(xres + 0.5);
        const int y = (int)(yres + 0.5);

        u32 dpi = (u32)y;

        return dpi;
    }

    int LinuxWindowMisc::GetWindowZOrder(SDLPlatformWindow* window)
    {
        if (window == nullptr)
            return -1;
        
        SDL_Window* sdlWindow = (SDL_Window*)window->GetUnderlyingHandle();
        if (sdlWindow == nullptr)
            return -1;

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (!SDL_GetWindowWMInfo(sdlWindow, &wmInfo)) 
            return -1;

        Display* display = wmInfo.info.x11.display;
        ::Window target = wmInfo.info.x11.window;

        ::Window root, parent;
        ::Window* children;
        unsigned int nchildren;

        if (XQueryTree(display, DefaultRootWindow(display), &root, &parent, &children, &nchildren)) 
        {
            // children[0] is bottom, children[n-1] is top
            for (int i = 0; i < (int)nchildren; ++i) 
            {
                if (children[i] == target) {
                    int zOrder = static_cast<int>(nchildren - 1 - i); // 0 = front, highest = back
                    XFree(children);
                    return zOrder;
                }
            }
            XFree(children);
        }
    }

}
