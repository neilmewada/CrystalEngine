
#include "CoreApplication.h"

#include <SDL.h>
#include <SDL_syswm.h>

namespace CE
{
	int SDLWindowEventWatch(void* data, SDL_Event* event);

	SDLApplication* SDLApplication::Get()
	{
		return (SDLApplication*)PlatformApplication::Get();
	}

	SDLApplication::SDLApplication()
	{
		
	}

	SDLApplication::~SDLApplication()
	{

	}

	void SDLApplication::Initialize()
	{
		Super::Initialize();

		SDL_SetHint(SDL_HINT_APP_NAME, gProjectName.GetCString());

		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
		SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "0");
		SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "0");

#if PLATFORM_MAC
		SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1");
#endif

#if PAL_TRAIT_BUILD_EDITOR
		// Hints for editor window
		SDL_SetHint("SDL_BORDERLESS_WINDOWED_STYLE", "1");
		SDL_SetHint("SDL_BORDERLESS_RESIZABLE_STYLE", "1");

		SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
		SDL_EventState(SDL_DROPTEXT, SDL_ENABLE);
#endif

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		{
			CE_LOG(Critical, All, "Failed to initialize SDL Video: {}", SDL_GetError());
		}

	}

	void SDLApplication::PreShutdown()
	{
		Super::PreShutdown();

		if (mainWindow != nullptr)
			DestroyWindow(mainWindow);
	}

	void SDLApplication::Shutdown()
	{
		Super::Shutdown();

		for (int i = 0; i < systemCursors.GetSize(); ++i)
		{
			if (systemCursors[i] != nullptr)
			{
				SDL_FreeCursor(systemCursors[i]);
				systemCursors[i] = nullptr;
			}
		}

		SDL_Quit();
	}

	f32 SDLApplication::GetSystemDpiScaling()
	{
#if PLATFORM_MAC
		return PlatformApplication::Get()->GetSystemDpi() / 72.0f;
#else
		return PlatformApplication::Get()->GetSystemDpi() / 96.0f;
#endif
	}

	bool SDLApplication::IsFocused()
	{
		for (auto window : windowList)
		{
			if (window->IsFocused())
				return true;
		}

		return false;
	}

	bool SDLApplication::SetMouseCapture(bool capture)
	{
		return SDL_CaptureMouse(capture ? SDL_TRUE : SDL_FALSE) == 0;
	}

	Rect SDLApplication::GetScreenBounds(int displayIndex)
	{
		SDL_Rect rect{};
		SDL_GetDisplayBounds(displayIndex, &rect);
		return Rect::FromSize(rect.x, rect.y, rect.w, rect.h);
	}

	void SDLApplication::SetSystemCursor(SystemCursor cursor)
	{
		if (systemCursors[(int)cursor] == nullptr)
		{
			systemCursors[(int)cursor] = SDL_CreateSystemCursor((SDL_SystemCursor)cursor);
		}

		SDL_SetCursor(systemCursors[(int)cursor]);
	}

	PlatformWindow* SDLApplication::InitMainWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen, bool resizable)
	{
		if (mainWindow == nullptr)
		{
			mainWindow = new SDLPlatformWindow(title, width, height, maximised, fullscreen, resizable);
			mainWindow->initialFlags |= PlatformWindowFlags::DestroyOnClose;
			mainWindow->isMainWindow = true;
			windowList.Add(mainWindow);

			SDL_AddEventWatch(SDLWindowEventWatch, mainWindow->handle);
		}

		for (auto messageHandler : messageHandlers)
		{
			messageHandler->OnWindowCreated(mainWindow);
		}
		return mainWindow;
	}

	PlatformWindow* SDLApplication::InitMainWindow(const String& title, u32 width, u32 height,
		const PlatformWindowInfo& info)
	{
		if (mainWindow == nullptr)
		{
			mainWindow = new SDLPlatformWindow(title, width, height, info);
			mainWindow->initialFlags |= PlatformWindowFlags::DestroyOnClose;
			mainWindow->isMainWindow = true;
			windowList.Add(mainWindow);

			SDL_AddEventWatch(SDLWindowEventWatch, mainWindow->handle);
		}

		for (auto messageHandler : messageHandlers)
		{
			messageHandler->OnWindowCreated(mainWindow);
		}
		return mainWindow;
	}

	PlatformWindow* SDLApplication::GetMainWindow()
	{
		return mainWindow;
	}

	PlatformWindow* SDLApplication::FindPlatformWindow(u64 windowId)
	{
		for (auto window : windowList)
		{
			if (window->GetWindowId() == windowId)
			{
				return window;
			}
		}

		return nullptr;
	}

	PlatformWindow* SDLApplication::CreatePlatformWindow(const String& title)
	{
		if (mainWindow == nullptr)
		{
			return InitMainWindow(title, gDefaultWindowWidth, gDefaultWindowHeight, false, false);
		}

		auto window = new SDLPlatformWindow(title, gDefaultWindowWidth, gDefaultWindowHeight, false, false);
		windowList.Add(window);
		for (auto messageHandler : messageHandlers)
		{
			messageHandler->OnWindowCreated(window);
		}
		return window;
	}

	PlatformWindow* SDLApplication::CreatePlatformWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen, bool hidden)
	{
		if (mainWindow == nullptr)
		{
			return InitMainWindow(title, width, height, maximised, fullscreen);
		}

		auto window = new SDLPlatformWindow(title, width, height, maximised, fullscreen, false, hidden);
		windowList.Add(window);
		for (auto messageHandler : messageHandlers)
		{
			messageHandler->OnWindowCreated(window);
		}
		return window;
	}

	PlatformWindow* SDLApplication::CreatePlatformWindow(const String& title, u32 width, u32 height,
		const PlatformWindowInfo& info)
	{
		if (mainWindow == nullptr)
		{
			return InitMainWindow(title, width, height, info);
		}

		auto window = new SDLPlatformWindow(title, width, height, info);
		windowList.Add(window);
        
		for (auto messageHandler : messageHandlers)
		{
			messageHandler->OnWindowCreated(window);
		}
		return window;
	}

	Vec2i SDLApplication::GetMainScreenSize()
	{
		return GetScreenSizeForWindow(mainWindow);
	}

	Vec2i SDLApplication::GetScreenSizeForWindow(PlatformWindow* window)
	{
		SDLPlatformWindow* sdlWindow = (SDLPlatformWindow*)window;

		SDL_DisplayMode mode{};
		int displayIndex = 0;
		if (sdlWindow != nullptr)
			displayIndex = SDL_GetWindowDisplayIndex(sdlWindow->handle);
		
		if (SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0)
		{
			CE_LOG(Error, All, "Failed to get screen size for window. Error: {}", SDL_GetError());
			return Vec2i();
		}
		return Vec2i(mode.w, mode.h);
	}

	Vec2i SDLApplication::GetWindowSize(void* nativeWindowHandle)
	{
		SDL_Window* window = (SDL_Window*)nativeWindowHandle;
		if (window == nullptr)
			return Vec2i();

		Vec2i result{};
		SDL_GetWindowSize(window, &result.width, &result.height);
		return result;
	}

	void SDLApplication::DestroyWindow(PlatformWindow* window)
	{
		if (window == nullptr)
			return;
		
		auto sdlWindow = (SDLPlatformWindow*)window;

		windowList.Remove(sdlWindow);
		
		if (sdlWindow->GetWindowId() == mainWindow->GetWindowId())
		{
			SDL_DelEventWatch(SDLWindowEventWatch, mainWindow->handle);

			// Destroy all windows (i.e. shutdown application) if main window is destroyed
			for (int i = windowList.GetSize() - 1; i >= 0; --i)
			{
				if (windowList[i] == mainWindow)
				{
					windowList.RemoveAt(i);
					continue;
				}
				DestroyWindow(windowList[i]);
			}

			RequestEngineExit("MAIN_WINDOW_CLOSED");
			this->mainWindow = nullptr;
		}

		for (ApplicationMessageHandler* handler : messageHandlers)
		{
			handler->OnWindowDestroyed(window);
		}

		delete sdlWindow;
	}

	void SDLApplication::Tick()
	{
		PlatformApplication::Tick();

		// Handle SDL Events
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			for (auto handler : messageHandlers)
			{
				if (handler != nullptr)
				{
					handler->ProcessNativeEvents(&sdlEvent);
				}
			}

			if (sdlEvent.type == SDL_QUIT)
			{
				RequestEngineExit("APP_QUIT");
				break;
			}
			
			if (sdlEvent.type == SDL_WINDOWEVENT)
			{
				ProcessWindowEvents(sdlEvent);
			}
			
			if (sdlEvent.type == SDL_KEYDOWN || sdlEvent.type == SDL_KEYUP ||
				sdlEvent.type == SDL_MOUSEMOTION || sdlEvent.type == SDL_MOUSEBUTTONDOWN || sdlEvent.type == SDL_MOUSEBUTTONUP || sdlEvent.type == SDL_MOUSEWHEEL)
			{
				ProcessInputEvents(sdlEvent);
			}
		}

		Super::Tick();
	}

	bool SDLApplication::HasClipboardText()
	{
		return SDL_HasClipboardText();
	}

	String SDLApplication::GetClipboardText()
	{
		char* cString = SDL_GetClipboardText();
		String string = cString;
		SDL_free(cString);
		return string;
	}

	void SDLApplication::SetClipboardText(const String& text)
	{
		SDL_SetClipboardText(text.GetCString());
	}

	void SDLApplication::ProcessWindowEvents(SDL_Event& event)
	{
		if (event.window.event == SDL_WINDOWEVENT_RESIZED)
		{
            if (event.window.windowID == mainWindow->GetWindowId())
            {
                ProcessWindowResizeEvent(mainWindow);
            }
            else
            {
                for (PlatformWindow* window : windowList)
                {
                    if (event.window.windowID == window->GetWindowId())
                    {
                        ProcessWindowResizeEvent((SDLPlatformWindow*)window);
                        break;
                    }
                }
            }
		}
		else if (event.window.event == SDL_WINDOWEVENT_MOVED)
		{
			for (PlatformWindow* window : windowList)
			{
				if (event.window.windowID == window->GetWindowId())
				{
					for (auto handler : messageHandlers)
					{
						handler->OnWindowMoved(window, event.window.data1, event.window.data2);
					}

					break;
				}
			}
		}
		else if (event.window.event == SDL_WINDOWEVENT_CLOSE) // Close a specific window
		{
			for (PlatformWindow* window : windowList)
			{
				if (event.window.windowID == window->GetWindowId())
				{
					if ((window->GetInitialFlags() & PlatformWindowFlags::DestroyOnClose) != 0)
					{
						DestroyWindow(window);
					}
					else
					{
						for (auto messageHandler : messageHandlers)
						{
							messageHandler->OnWindowClosed(window);
						}
					}
					break;
				}
			}
		}
		else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
		{
			for (auto window : windowList)
			{
				if ((u32)window->GetWindowId() == event.window.windowID) // Found the window
				{
					for (ApplicationMessageHandler* handler : messageHandlers)
					{
						handler->OnWindowMinimized(window);
					}
					break;
				}
			}
		}
		else if (event.window.event == SDL_WINDOWEVENT_SHOWN)
		{
			for (auto window : windowList)
			{
				if ((u32)window->GetWindowId() == event.window.windowID) // Found the window
				{
					for (ApplicationMessageHandler* handler : messageHandlers)
					{
						handler->OnWindowShown(window);
					}
					break;
				}
			}
		}
		else if (event.window.event == SDL_WINDOWEVENT_RESTORED)
		{
			for (auto window : windowList)
			{
				if ((u32)window->GetWindowId() == event.window.windowID) // Found the window
				{
					for (ApplicationMessageHandler* handler : messageHandlers)
					{
						handler->OnWindowRestored(window);
					}
					break;
				}
			}
		}
		else if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
		{
			for (auto window : windowList)
			{
				if ((u32)window->GetWindowId() == event.window.windowID) // Found the window
				{
					for (ApplicationMessageHandler* handler : messageHandlers)
					{
						handler->OnWindowMaximized(window);
					}
					break;
				}
			}
		}
	}

	void SDLApplication::ProcessInputEvents(SDL_Event& event)
	{
		for (auto handler : messageHandlers)
		{
			handler->ProcessInputEvents(&event);
		}
	}

	void SDLApplication::ProcessWindowResizeEvent(SDLPlatformWindow* window)
	{
		u32 w = 0, h = 0;
        window->GetDrawableWindowSize(&w, &h);

		if (w != 0 && h != 0)
		{
			for (auto handler : messageHandlers)
			{
				if (handler != nullptr && window->GetWindowId() == mainWindow->GetWindowId())
					handler->OnMainWindowDrawableSizeChanged(w, h);
			}
            onWindowDrawableSizeChanged.Broadcast(window, w, h);

			for (ApplicationMessageHandler* handler : messageHandlers)
			{
				handler->OnWindowResized(window, w, h);
			}
		}
	}

	void SDLApplication::ProcessWindowMoveEvent(SDLPlatformWindow* window)
	{
		
	}

}
