#pragma once

#include "ApplicationMessageHandler.h"

namespace CE
{
    class PlatformWindow;

    typedef MultiCastDelegate<void(PlatformWindow* window, u32 newWidth, u32 newHeight)> WindowResizeDelegate;

    enum class PlatformBackend
    {
        None = 0,
        SDL
    };

    enum class SystemCursor
    {
        Default = 0,
        Arrow = 0,
        IBeam,
        Wait,
        CrossHair,
        WaitArrow,
        SizeTopLeft,
        SizeBottomRight = SizeTopLeft,
        SizeTopRight,
        SizeBottomLeft = SizeTopRight,
        SizeHorizontal,
        SizeVertical,
        SizeAll,
        No,
        Hand,
        COUNT
    };
    ENUM_CLASS(SystemCursor);

    enum class PlatformWindowFlags
    {
        None = 0,
        SkipTaskbar = BIT(0),
        ToolTip = BIT(1),
        PopupMenu = BIT(2),
        Utility = BIT(3),
        DestroyOnClose = BIT(4)
    };
    ENUM_CLASS_FLAGS(PlatformWindowFlags);

    struct PlatformWindowInfo
    {
        bool maximised = false;
    	bool fullscreen = false;
    	bool resizable = true;
        bool hidden = false;
        // -1 means automatic, 0 means first display.
        int displayIndex = -1;
        bool openCentered = true;
        Vec2i openPos = Vec2i();
        PlatformWindowFlags windowFlags = PlatformWindowFlags::DestroyOnClose;
    };

    class COREAPPLICATION_API PlatformApplication
    {
    public:

        static PlatformApplication* Get();

        static PlatformApplication* TryGet();

        PlatformApplication() = default;

        virtual ~PlatformApplication();

        virtual u32 GetSystemDpi() = 0;

        virtual f32 GetSystemDpiScaling() = 0;

        virtual void AddMessageHandler(ApplicationMessageHandler* handler)
        {
            this->messageHandlers.Add(handler);
        }

        virtual void RemoveMessageHandler(ApplicationMessageHandler* handler)
        {
            this->messageHandlers.Remove(handler);
        }

        virtual DelegateHandle AddTickHandler(const Delegate<void(void)> tickHandler)
        {
            this->tickHanders.Add(tickHandler);
            return tickHandler.GetHandle();
        }

        virtual void RemoveTickHandler(DelegateHandle tickHandle)
        {
            this->tickHanders.RemoveAll([&](const Delegate<void()>& del) { return del.GetHandle() == tickHandle; });
        }

        virtual const Array<ApplicationMessageHandler*>& GetMessageHandlers() const
        {
            return messageHandlers;
        }

        virtual void Initialize();
        virtual void PreShutdown();
        virtual void Shutdown();

        virtual void Tick();

        virtual bool IsFocused() = 0;

        virtual PlatformBackend GetBackend() = 0;

        virtual Rect GetScreenBounds(int displayIndex = 0) = 0;

        virtual void SetSystemCursor(SystemCursor cursor) = 0;
        
        virtual int GetCurrentDisplayIndex() { return 0; }

        virtual PlatformWindow* InitMainWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen, bool resizable = true) = 0;

        virtual PlatformWindow* InitMainWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info) = 0;

        virtual PlatformWindow* GetMainWindow() = 0;

        virtual PlatformWindow* FindPlatformWindow(u64 windowId) = 0;

        virtual PlatformWindow* CreatePlatformWindow(const String& title) = 0;
        virtual PlatformWindow* CreatePlatformWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen = false, bool hidden = false) = 0;

        virtual PlatformWindow* CreatePlatformWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info) = 0;

        virtual void DestroyWindow(PlatformWindow* window) = 0;

		virtual Vec2i GetMainScreenSize() = 0;
		virtual Vec2i GetScreenSizeForWindow(PlatformWindow* window) = 0;

		virtual Vec2i GetWindowSize(void* nativeWindowHandle) = 0;

        // Clipboard

        virtual bool HasClipboardText() = 0;
        virtual String GetClipboardText() = 0;
        virtual void SetClipboardText(const String& text) = 0;

        WindowResizeDelegate onWindowDrawableSizeChanged{};

    protected:

        static PlatformApplication* instance;

        Array<ApplicationMessageHandler*> messageHandlers{};
        Array<Delegate<void(void)>> tickHanders{};
    };

} // namespace CE
