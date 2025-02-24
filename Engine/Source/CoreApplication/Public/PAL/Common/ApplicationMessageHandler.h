#pragma once

namespace CE
{

    class PlatformWindow;

    class ApplicationMessageHandler
    {
    public:

        virtual ~ApplicationMessageHandler() = default;

        virtual void OnMainWindowDrawableSizeChanged(u32 width, u32 height)
        {

        }

        virtual void ProcessNativeEvents(void* nativeEvent)
        {

        }

        virtual void ProcessInputEvents(void* nativeEvent)
        {

        }

        virtual void OnWindowShown(PlatformWindow* window) {}
        virtual void OnWindowMaximized(PlatformWindow* window) {}
        virtual void OnWindowRestored(PlatformWindow* window) {}
        virtual void OnWindowDestroyed(PlatformWindow* window) {}
        virtual void OnWindowClosed(PlatformWindow* window) {}
        virtual void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) {}
        virtual void OnWindowMoved(PlatformWindow* window, int x, int y) {}
        virtual void OnWindowMinimized(PlatformWindow* window) {}
        virtual void OnWindowCreated(PlatformWindow* window) {}
        virtual void OnWindowExposed(PlatformWindow* window) {}

    };
    
} // namespace CE

