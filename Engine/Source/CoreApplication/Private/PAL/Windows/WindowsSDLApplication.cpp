#include "CoreApplication.h"

#include <Windows.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

namespace CE
{
    WindowsSDLApplication* WindowsSDLApplication::Create()
    {
        return new WindowsSDLApplication();
    }

    void WindowsSDLApplication::Initialize()
    {
        Super::Initialize();
    }

    u32 WindowsSDLApplication::GetSystemDpi()
    {
        return GetDpiForSystem();
    }

    WindowsSDLApplication::WindowsSDLApplication()
    {

    }

	bool SDLWindowEventWatch(void* data, SDL_Event* event)
	{
		if (event->type == SDL_EVENT_WINDOW_EXPOSED)
		{
			auto app = SDLApplication::Get();

			for (SDLPlatformWindow* window : app->windowList)
			{
				if (window->GetWindowId() == event->window.windowID)
				{
					for (ApplicationMessageHandler* messageHandler : app->messageHandlers)
					{
						messageHandler->OnWindowExposed(window);
					}

					break;
				}
			}

			for (const auto& tickHandler : app->tickHanders)
			{
				tickHandler.InvokeIfValid();
			}
		}

		return true;
	}

}
