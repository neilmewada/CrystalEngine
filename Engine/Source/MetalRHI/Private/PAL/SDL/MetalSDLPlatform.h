#pragma once

#include "SDL.h"
#include "SDL_metal.h"

#include "CoreApplication.h"

namespace CE::Metal
{
    
    class MetalSDLPlatform : public MetalOSPlatform
    {
        
    public:

        static bool IsValidationEnabled()
        {
#if CE_BUILD_DEBUG
            return true;
#else
            return false;
#endif
        }

        static bool IsVerboseValidationEnabled()
        {
#if CE_BUILD_DEBUG
            return true;
#else
            return false;
#endif
        }

        static PlatformWindow* GetMainPlatformWindow()
        {
            if (PlatformApplication::Get()->GetMainWindow() == nullptr)
                return nullptr;
            return PlatformApplication::Get()->GetMainWindow();
        }

		static Vec2i GetScreenSizeForWindow(void* sdlWindow);

        static void InitMetalForWindow(PlatformWindow* window);

        static void ShutdownMetalForWindow();
        
        static CAMetalLayer* GetCAMetalLayer(PlatformWindow* window);
    };

    typedef MetalSDLPlatform MetalPlatform;

} // namespace CE
