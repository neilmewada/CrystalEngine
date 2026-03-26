
#include "CoreApplication.h"

#include "PAL/Common/PlatformWindowMisc.h"

#include <SDL3/SDL.h>

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

        NSWindow* nsWindow = (__bridge NSWindow*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(handle),
            SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,
            nullptr
        );
        
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

        SDL_SetWindowBordered(handle, !borderless);
        return;

        NSWindow* nsWindow = (__bridge NSWindow*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(handle),
            SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,
            nullptr
        );
        
        NSWindowStyleMask styleMask = [nsWindow styleMask];
        styleMask |= NSWindowStyleMaskFullSizeContentView;
        
        [nsWindow setStyleMask:(styleMask & ~NSWindowStyleMaskBorderless)];
        [nsWindow setTitlebarAppearsTransparent:YES];
        [nsWindow setTitleVisibility:NSWindowTitleHidden];
        [nsWindow setShowsToolbarButton:YES];
    }

    int MacWindowMisc::GetWindowZOrder(SDLPlatformWindow* window)
    {
        NSWindow* nsWindow = (__bridge NSWindow*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(window->GetSdlHandle()),
            SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,
            nullptr
        );

        NSArray* windows = [NSApp orderedWindows];
        NSInteger zOrder = [windows indexOfObject:nsWindow];

        return zOrder;
    }

} // namespace CE
