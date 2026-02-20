#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "vulkan/vulkan.h"
//#include "vma/vk_mem_alloc.h"

#include "CoreApplication.h"

namespace CE::Vulkan
{
    
    class VulkanSDLPlatform : public VulkanOSPlatform
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

        static Array<const char*> GetRequiredInstanceExtensions()
        {
            auto extensions = VulkanOSPlatform::GetRequiredInstanceExtensions();
#if PLATFORM_DESKTOP && !CE_BUILD_RELEASE
            extensions.AddRange({
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
            });
#else
            
#endif
            
            return extensions;
        }

        static Array<const char*> GetValidationLayers()
        {
            return
            {
                "VK_LAYER_KHRONOS_validation"
            };
        }

        static PlatformWindow* GetMainPlatformWindow()
        {
            if (PlatformApplication::Get()->GetMainWindow() == nullptr)
                return nullptr;
            return PlatformApplication::Get()->GetMainWindow();
        }

        static VkSurfaceKHR CreateSurface(VkInstance vkInstance, PlatformWindow* platformWindow)
        {
            return platformWindow->CreateVulkanSurface(vkInstance);
        }

        static void DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface)
        {
            vkDestroySurfaceKHR(vkInstance, surface, nullptr);
        }

        CE_DEPRECATED("Do not use any SDL functions in VulkanRHI library. Use CoreApplication APIs instead")
        static void GetDrawableWindowSize(void* sdlWindow, u32* width, u32* height)
        {
            int w = 0, h = 0;
			SDL_GetWindowSizeInPixels((SDL_Window*)sdlWindow, &w, &h);
            *width = (u32)w;
            *height = (u32)h;
        }

		static Vec2i GetScreenSizeForWindow(void* sdlWindow);

        static void InitVulkanForWindow(PlatformWindow* window);

        static void ShutdownVulkanForWindow();
    };

    typedef VulkanSDLPlatform VulkanPlatform;

} // namespace CE
