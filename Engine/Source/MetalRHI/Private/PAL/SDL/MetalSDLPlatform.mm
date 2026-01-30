
#include "PAL/Common/MetalPlatform.h"

namespace CE::Metal
{
	Vec2i MetalSDLPlatform::GetScreenSizeForWindow(void* sdlWindowHandle)
	{
		SDL_Window* sdlWindow = (SDL_Window*)sdlWindowHandle;

		SDL_DisplayMode mode{};
		int displayIndex = 0;
		if (sdlWindow != nullptr)
			displayIndex = SDL_GetWindowDisplayIndex(sdlWindow);
		if (SDL_GetCurrentDisplayMode(displayIndex, &mode) != 0)
		{
			CE_LOG(Error, All, "Failed to get screen size for window. Error: {}", SDL_GetError());
			return Vec2i();
		}
		return Vec2i(mode.w, mode.h);
	}

	void MetalSDLPlatform::InitMetalForWindow(PlatformWindow* window)
	{
		auto sdlWindow = (SDL_Window*)((SDLPlatformWindow*)window)->GetUnderlyingHandle();
	}

	void MetalSDLPlatform::ShutdownMetalForWindow()
	{
		
	}

    CAMetalLayer* MetalSDLPlatform::GetCAMetalLayer(PlatformWindow *window)
    {
        SDLPlatformWindow* sdlWindow = (SDLPlatformWindow*)window;
        
        return (__bridge CAMetalLayer*)SDL_Metal_GetLayer((SDL_MetalView)sdlWindow->GetViewHandle());
    }

} // namespace CE

