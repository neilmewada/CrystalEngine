
#include "PAL/Common/VulkanPlatform.h"

namespace CE::Vulkan
{
	Vec2i VulkanSDLPlatform::GetScreenSizeForWindow(void* sdlWindowHandle)
	{
		SDL_Window* sdlWindow = (SDL_Window*)sdlWindowHandle;

		DisplayId displayId = 0;
		if (sdlWindow != nullptr)
			displayId = SDL_GetDisplayForWindow(sdlWindow);

		const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayId);

		if (mode == nullptr)
		{
			CE_LOG(Error, All, "Failed to get screen size for window. Error: {}", SDL_GetError());
			return Vec2i();
		}
		return Vec2i(mode->w, mode->h);
	}

	void VulkanSDLPlatform::InitVulkanForWindow(PlatformWindow* window)
	{
		auto sdlWindow = (SDL_Window*)((SDLPlatformWindow*)window)->GetUnderlyingHandle();
	}

	void VulkanSDLPlatform::ShutdownVulkanForWindow()
	{
		
	}
	
} // namespace CE

