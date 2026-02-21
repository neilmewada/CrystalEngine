
#include "CoreApplication.h"

#include <SDL3/SDL.h>
#if PAL_TRAIT_VULKAN_SUPPORTED
#include <SDL3/SDL_vulkan.h>
#endif

#if PAL_TRAIT_METAL_SUPPORTED
#include <SDL3/SDL_metal.h>
#endif

#include <SDL3/SDL_system.h>

#include <algorithm>

#include "PAL/Common/PlatformWindowMisc.h"

constexpr auto MouseGrabPadding = 8;
constexpr auto WindowDragPadding = 50;

namespace CE
{
	SDLPlatformWindow::~SDLPlatformWindow()
	{
#if PAL_TRAIT_METAL_SUPPORTED
        SDL_Metal_DestroyView(metalView);
        metalView = nullptr;
#endif
        
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
		u32 flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
		if (resizable)
			flags |= SDL_WINDOW_RESIZABLE;
		if (isHidden)
			flags |= SDL_WINDOW_HIDDEN;
#if PAL_TRAIT_METAL_SUPPORTED
        flags |= SDL_WINDOW_METAL;
#elif PAL_TRAIT_VULKAN_SUPPORTED
		flags |= SDL_WINDOW_VULKAN;
#endif
		if (maximised)
			flags |= SDL_WINDOW_MAXIMIZED;
		if (fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN;

		handle = SDL_CreateWindow(title.GetCString(), width, height, flags);
		SDL_SetWindowPosition(handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        
#if PAL_TRAIT_METAL_SUPPORTED
        metalView = SDL_Metal_CreateView(handle);
#endif
	}

	SDLPlatformWindow::SDLPlatformWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info)
		: initialFlags(info.windowFlags)
	{
		u32 flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
		if (info.resizable)
			flags |= SDL_WINDOW_RESIZABLE;
		if (info.hidden)
			flags |= SDL_WINDOW_HIDDEN;
#if PAL_TRAIT_METAL_SUPPORTED
        flags |= SDL_WINDOW_METAL;
#elif PAL_TRAIT_VULKAN_SUPPORTED
        flags |= SDL_WINDOW_VULKAN;
#endif
		if (info.maximised)
			flags |= SDL_WINDOW_MAXIMIZED;
		if (info.fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::PopupMenu))
			flags |= SDL_WINDOW_POPUP_MENU;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::ToolTip))
			flags |= SDL_WINDOW_TOOLTIP;
		if (EnumHasFlag(info.windowFlags, PlatformWindowFlags::Utility))
			flags |= SDL_WINDOW_UTILITY;

		int x = SDL_WINDOWPOS_CENTERED;
		int y = SDL_WINDOWPOS_CENTERED;

		if (!info.openCentered)
		{
			x = info.openPos.x;
			y = info.openPos.y;
		}

        handle = SDL_CreateWindow(title.GetCString(), width, height, flags);
		SDL_SetWindowPosition(handle, x, y);

#if PAL_TRAIT_METAL_SUPPORTED
        metalView = SDL_Metal_CreateView(handle);
#endif
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
		SDL_SetWindowAlwaysOnTop(handle, alwaysOnTop);
	}

	void SDLPlatformWindow::GetDrawableWindowSize(u32* outWidth, u32* outHeight)
	{
		int w = 0, h = 0;
		SDL_GetWindowSizeInPixels(handle, &w, &h);
		*outWidth = (u32)w;
		*outHeight = (u32)h;
	}

	Vec2i SDLPlatformWindow::GetDrawableWindowSize()
	{
		int w = 0, h = 0;
		SDL_GetWindowSizeInPixels(handle, &w, &h);
		return Vec2i(w, h);
	}

	void SDLPlatformWindow::SetResizable(bool resizable)
	{
		SDL_SetWindowResizable(handle, resizable);
	}

	void SDLPlatformWindow::SetBorderless(bool borderless)
	{
		SDL_SetWindowBordered(handle, !borderless);

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
		return !IsHidden();
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
#if PAL_TRAIT_VULKAN_SUPPORTED
		SDL_Vulkan_CreateSurface(handle, instance, nullptr, &surface);
#endif
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
		SDL_PropertiesID props = SDL_GetWindowProperties(handle);

#if PLATFORM_WINDOWS
		return (WindowHandle)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif PLATFORM_MAC
		return (WindowHandle)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif PLATFORM_LINUX

#if defined(SDL_VIDEO_DRIVER_X11)
		{
			Window xWindow = (Window)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
			if (xWindow)
				return (WindowHandle)xWindow;
		}
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
		{
			void* wlWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER, nullptr);
			if (wlWindow)
				return (WindowHandle)wlWindow;
		}
#endif
#endif
		return 0;
	}

    void* SDLPlatformWindow::GetViewHandle()
    {
#if PAL_TRAIT_METAL_SUPPORTED
        return metalView;
#endif
        return nullptr;
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

	f32 SDLPlatformWindow::GetDpiScaling()
	{
		return SDL_GetWindowDisplayScale(handle);
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

