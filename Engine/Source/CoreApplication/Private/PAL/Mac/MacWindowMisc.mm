
#include "CoreApplication.h"

#include "PAL/Common/PlatformWindowMisc.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <AppKit/AppKit.h>

namespace CE
{

    u32 MacWindowMisc::GetDpiForWindow(SDLPlatformWindow* window)
    {
        if (window == nullptr)
        {
            return PlatformApplication::Get()->GetSystemDpi();
        }

        auto handle = (SDL_Window*)window->GetUnderlyingHandle();

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(handle, &wmInfo);
        
        NSWindow* nsWindow = wmInfo.info.cocoa.window;
        
        // Get the backing scale factor
        CGFloat scaleFactor = [nsWindow backingScaleFactor]; // Alternatively, screen.backingScaleFactor

        // Calculate DPI
        CGFloat logicalDPI = 72.0; // macOS logical DPI
        CGFloat effectiveDPI = logicalDPI * scaleFactor;
        
        return (u32)effectiveDPI;
    }

    void MacWindowMisc::SetupBorderlessWindow(SDLPlatformWindow* window, bool borderless)
    {
        if (window == nullptr)
        {
            return;
        }
        
        auto handle = (SDL_Window*)window->GetUnderlyingHandle();
        
        SDL_SetWindowBordered(handle, SDL_FALSE);

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(handle, &wmInfo);
        
        NSWindow* nsWindow = wmInfo.info.cocoa.window;
        
        NSWindowStyleMask styleMask = [nsWindow styleMask];
        styleMask |= NSWindowStyleMaskFullSizeContentView;
        
        [nsWindow setStyleMask:(styleMask & ~NSWindowStyleMaskBorderless)];
        [nsWindow setTitlebarAppearsTransparent:YES];
        [nsWindow setTitleVisibility:NSWindowTitleHidden];
        [nsWindow setShowsToolbarButton:YES];
    }

    int MacWindowMisc::GetWindowZOrder(SDLPlatformWindow* window)
    {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (!SDL_GetWindowWMInfo(window->GetSdlHandle(), &wmInfo))
            return -1;

        NSWindow* nsWindow = wmInfo.info.cocoa.window;

        NSArray* windows = [NSApp orderedWindows];
        NSInteger zOrder = [windows indexOfObject:nsWindow];

        return zOrder;
    }

} // namespace CE
