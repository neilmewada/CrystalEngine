
#include "CoreApplication.h"

#include <SDL.h>
#if PAL_TRAIT_VULKAN_SUPPORTED
#include <SDL_vulkan.h>
#endif

#include <SDL_syswm.h>

#include "PAL/Common/PlatformWindowMisc.h"

constexpr auto MouseGrabPadding = 8;
constexpr auto WindowDragPadding = 50;

namespace CE
{
	SDLPlatformWindow::~SDLPlatformWindow()
	{
		if (handle != nullptr)
			SDL_DestroyWindow(handle);
		handle = nullptr;
	}

	// Credit: https://gist.github.com/dele256/901dd1e8f920327fc457a538996f2a29
	static SDL_HitTestResult HitTestCallback(SDL_Window* window, const SDL_Point* area, void* data)
	{
		int Width, Height;
		SDL_GetWindowSize(window, &Width, &Height);

		SDLPlatformWindow* platformWindow = (SDLPlatformWindow*)data;
		
		if (platformWindow->IsResizable())
		{
			if (area->y < MouseGrabPadding)
			{
				if (area->x < MouseGrabPadding)
				{
					return SDL_HITTEST_RESIZE_TOPLEFT;
				}
				else if (area->x > Width - MouseGrabPadding)
				{
					return SDL_HITTEST_RESIZE_TOPRIGHT;
				}
				else
				{
					return SDL_HITTEST_RESIZE_TOP;
				}
			}
			else if (area->y > Height - MouseGrabPadding)
			{
				if (area->x < MouseGrabPadding)
				{
					return SDL_HITTEST_RESIZE_BOTTOMLEFT;
				}
				else if (area->x > Width - MouseGrabPadding)
				{
					return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
				}
				else
				{
					return SDL_HITTEST_RESIZE_BOTTOM;
				}
			}
			else if (area->x < MouseGrabPadding)
			{
				return SDL_HITTEST_RESIZE_LEFT;
			}
			else if (area->x > Width - MouseGrabPadding)
			{
				return SDL_HITTEST_RESIZE_RIGHT;
			}
		}

		if (platformWindow->dragHitTest.IsValid())
		{
			bool result = platformWindow->dragHitTest.Invoke(platformWindow, Vec2(area->x, area->y));
			return result ? SDL_HITTEST_DRAGGABLE : SDL_HITTEST_NORMAL;
		}
		else if (area->y < WindowDragPadding)
		{
			return SDL_HITTEST_DRAGGABLE;
		}
		
		return SDL_HITTEST_NORMAL; // SDL_HITTEST_NORMAL <- Windows behaviour
	}

	SDLPlatformWindow::SDLPlatformWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen, bool resizable, bool isHidden, int displayIndex)
	{
		u32 flags = SDL_WINDOW_ALLOW_HIGHDPI;
		if (resizable)
			flags |= SDL_WINDOW_RESIZABLE;
		if (isHidden)
			flags |= SDL_WINDOW_HIDDEN;
#if PAL_TRAIT_VULKAN_SUPPORTED
		flags |= SDL_WINDOW_VULKAN;
#endif
		if (maximised)
			flags |= SDL_WINDOW_MAXIMIZED;
		if (fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        
        if (displayIndex == -1)
        {
            SDLPlatformWindow* mainWindow = (SDLPlatformWindow*)PlatformApplication::Get()->GetMainWindow();
            if (mainWindow == nullptr)
            {
                displayIndex = PlatformApplication::Get()->GetCurrentDisplayIndex();
            }
            else
            {
                displayIndex = SDL_GetWindowDisplayIndex(mainWindow->handle);
            }
        }
        
        if (displayIndex < 0)
        {
            displayIndex = 0;
        }

		handle = SDL_CreateWindow(title.GetCString(), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), width, height, flags);
	}

	SDLPlatformWindow::SDLPlatformWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info)
		: initialFlags(info.windowFlags)
	{
		u32 flags = SDL_WINDOW_ALLOW_HIGHDPI;
		if (info.resizable)
			flags |= SDL_WINDOW_RESIZABLE;
		if (info.hidden)
			flags |= SDL_WINDOW_HIDDEN;
#if PAL_TRAIT_VULKAN_SUPPORTED
		flags |= SDL_WINDOW_VULKAN;
#endif
		if (info.maximised)
			flags |= SDL_WINDOW_MAXIMIZED;
		if (info.fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::SkipTaskbar))
			flags |= SDL_WINDOW_SKIP_TASKBAR;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::PopupMenu))
			flags |= SDL_WINDOW_POPUP_MENU;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::ToolTip))
			flags |= SDL_WINDOW_TOOLTIP;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::Utility))
			flags |= SDL_WINDOW_UTILITY;
        
        int displayIndex = info.displayIndex;
        if (displayIndex == -1)
        {
            SDLPlatformWindow* mainWindow = (SDLPlatformWindow*)PlatformApplication::Get()->GetMainWindow();
            if (mainWindow == nullptr)
            {
                displayIndex = PlatformApplication::Get()->GetCurrentDisplayIndex();
            }
            else
            {
                displayIndex = SDL_GetWindowDisplayIndex(mainWindow->handle);
            }
        }

		displayIndex = Math::Max(displayIndex, 0);

		int x = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);
		int y = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);

		if (!info.openCentered)
		{
			x = info.openPos.x;
			y = info.openPos.y;
		}

        handle = SDL_CreateWindow(title.GetCString(), x, y, width, height, flags);
	}

	void SDLPlatformWindow::GetWindowSize(u32* outWidth, u32* outHeight)
	{
		int w = 0, h = 0;
		SDL_GetWindowSize(handle, &w, &h);
		*outWidth = (u32)w;
		*outHeight = (u32)h;
	}

	Vec2i SDLPlatformWindow::GetWindowSize()
	{
		Vec2i size{};
		SDL_GetWindowSize(handle, &size.width, &size.height);
		return size;
	}

	void SDLPlatformWindow::SetWindowSize(Vec2i newSize)
	{
		SDL_SetWindowSize(handle, newSize.x, newSize.y);
	}

	void SDLPlatformWindow::SetMinimumSize(Vec2i minSize)
	{
		SDL_SetWindowMinimumSize(handle, minSize.width, minSize.height);
	}

	void SDLPlatformWindow::SetOpacity(f32 opacity)
	{
		SDL_SetWindowOpacity(handle, opacity);
	}

	void SDLPlatformWindow::SetAlwaysOnTop(bool alwaysOnTop)
	{
		SDL_SetWindowAlwaysOnTop(handle, alwaysOnTop ? SDL_TRUE : SDL_FALSE);
	}

	void SDLPlatformWindow::GetDrawableWindowSize(u32* outWidth, u32* outHeight)
	{
#if PAL_TRAIT_VULKAN_SUPPORTED
		int w = 0, h = 0;
		SDL_Vulkan_GetDrawableSize(handle, &w, &h);
		*outWidth = (u32)w;
		*outHeight = (u32)h;
#else
		GetWindowSize(outWidth, outHeight);
		return;
#endif
	}

	Vec2i SDLPlatformWindow::GetDrawableWindowSize()
	{
		int w = 0, h = 0;
#if PAL_TRAIT_VULKAN_SUPPORTED
		SDL_Vulkan_GetDrawableSize(handle, &w, &h);
		return Vec2i(w, h);
#else
		SDL_GetWindowSize(handle, &w, &h);
		return Vec2i(w, h);
#endif
	}

	void SDLPlatformWindow::SetResizable(bool resizable)
	{
		SDL_SetWindowResizable(handle, resizable ? SDL_TRUE : SDL_FALSE);
	}

	void SDLPlatformWindow::SetBorderless(bool borderless)
	{
		SDL_SetWindowBordered(handle, borderless ? SDL_FALSE : SDL_TRUE);

		if (borderless)
		{
			SDL_SetWindowHitTest(handle, HitTestCallback, this);
		}
		else
		{
			SDL_SetWindowHitTest(handle, nullptr, nullptr);
		}
        
        PlatformWindowMisc::SetupBorderlessWindow(this, borderless);
	}

	void SDLPlatformWindow::RaiseWindow()
	{
		SDL_RaiseWindow(handle);
		//SDL_SetWindowInputFocus(handle);
	}

	bool SDLPlatformWindow::IsBorderless()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_BORDERLESS) != 0;
	}

	bool SDLPlatformWindow::IsMinimized()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_MINIMIZED) != 0;
	}

	bool SDLPlatformWindow::IsMaximized()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_MAXIMIZED) != 0;
	}

	bool SDLPlatformWindow::IsFullscreen()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_FULLSCREEN) != 0;
	}

	bool SDLPlatformWindow::IsShown()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_SHOWN) != 0;
	}

	bool SDLPlatformWindow::IsHidden()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_HIDDEN) != 0;
	}

	bool SDLPlatformWindow::IsAlwaysOnTop()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_ALWAYS_ON_TOP) != 0;
	}

	bool SDLPlatformWindow::IsResizable()
	{
		return (SDL_GetWindowFlags(handle) & SDL_WINDOW_RESIZABLE) != 0;
	}

	void SDLPlatformWindow::Minimize()
	{
		SDL_MinimizeWindow(handle);
	}

	void SDLPlatformWindow::Restore()
	{
		SDL_RestoreWindow(handle);
	}

	void SDLPlatformWindow::Maximize()
	{
		SDL_MaximizeWindow(handle);
	}

	VkSurfaceKHR SDLPlatformWindow::CreateVulkanSurface(VkInstance instance)
	{
		VkSurfaceKHR surface = nullptr;
		SDL_Vulkan_CreateSurface(handle, instance, &surface);
		return surface;
	}

	u64 SDLPlatformWindow::GetWindowId()
	{
		return SDL_GetWindowID(handle);
	}

	void* SDLPlatformWindow::GetUnderlyingHandle()
	{
		return handle;
	}

	WindowHandle SDLPlatformWindow::GetOSNativeHandle()
	{
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(handle, &wmInfo);

#if PLATFORM_WINDOWS
		return (WindowHandle)wmInfo.info.win.window;
#elif PLATFORM_MAC
        return (WindowHandle)wmInfo.info.cocoa.window;
#elif PLATFORM_LINUX
		if (wmInfo.subsystem == SDL_SYSWM_X11)
			return wmInfo.info.x11.window;
		if (wmInfo.subsystem == SDL_SYSWM_WAYLAND)
			return (WindowHandle)wmInfo.info.wl.egl_window;
		return 0;
#else
#   error Platform specific window handle not specified for the current platform
#endif
	}

	String SDLPlatformWindow::GetTitle()
	{
		return SDL_GetWindowTitle(handle);
	}

	int SDLPlatformWindow::GetZOrder()
	{
		return PlatformWindowMisc::GetWindowZOrder(this);
	}

	u32 SDLPlatformWindow::GetWindowDpi()
	{
		return PlatformWindowMisc::GetDpiForWindow(this);
	}

	void SDLPlatformWindow::Show()
	{
		SDL_ShowWindow(handle);
	}

	void SDLPlatformWindow::Hide()
	{
		SDL_HideWindow(handle);
	}

	bool SDLPlatformWindow::IsFocused()
	{
		Uint32 flags = SDL_GetWindowFlags(handle);
		return (flags & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS)) != 0;
	}

	Vec2i SDLPlatformWindow::GetWindowPosition()
	{
		int x;  int y;
		SDL_GetWindowPosition(handle, &x, &y);
		return Vec2i(x, y);
	}

	void SDLPlatformWindow::SetWindowPosition(Vec2i position)
	{
		SDL_SetWindowPosition(handle, position.x, position.y);
	}

} // namespace CE

