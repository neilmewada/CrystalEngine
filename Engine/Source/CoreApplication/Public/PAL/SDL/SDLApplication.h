#pragma once

union SDL_Event;
struct SDL_Cursor;

namespace CE
{
    class SDLPlatformWindow;

    class COREAPPLICATION_API SDLApplication : public PlatformApplication
    {
    public:
        typedef PlatformApplication Super;

        static SDLApplication* Get();

        virtual ~SDLApplication();

        void Initialize() override;

        void PreShutdown() override;

        void Shutdown() override;

        f32 GetSystemDpiScaling() override;

        PlatformBackend GetBackend() override
        {
            return PlatformBackend::SDL;
        }

        virtual bool IsFocused() override;

        virtual bool SetMouseCapture(bool capture) override;

        virtual Rect GetScreenBounds(int displayIndex = 0) override;

        virtual void SetSystemCursor(SystemCursor cursor) override;

        PlatformWindow* InitMainWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen, bool resizable = true) override;

        PlatformWindow* InitMainWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info) override;

        PlatformWindow* GetMainWindow() override;

        virtual PlatformWindow* FindPlatformWindow(u64 windowId) override;

        virtual PlatformWindow* CreatePlatformWindow(const String& title) override;
        virtual PlatformWindow* CreatePlatformWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen = false, bool hidden = false) override;

        virtual PlatformWindow* CreatePlatformWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info) override;

		virtual Vec2i GetMainScreenSize() override;
		virtual Vec2i GetScreenSizeForWindow(PlatformWindow* window) override;

		virtual Vec2i GetWindowSize(void* nativeWindowHandle) override;

        void DestroyWindow(PlatformWindow* window) override;

        void Tick() override;

        bool HasClipboardText() override;
        String GetClipboardText() override;
        void SetClipboardText(const String& text) override;

    public:
        Mutex mutex{};

        bool mainWindowForceResizing = false;

    protected:

        void ProcessWindowEvents(SDL_Event& event);

        void ProcessInputEvents(SDL_Event& event);

        void ProcessWindowResizeEvent(SDLPlatformWindow* window);

        void ProcessWindowMoveEvent(SDLPlatformWindow* window);

        SDLApplication();

        SDLPlatformWindow* mainWindow = nullptr;

        Array<SDLPlatformWindow*> windowList{};

        friend int SDLWindowEventWatch(void* data, SDL_Event* event);

    private:

        StaticArray<SDL_Cursor*, (SIZE_T)SystemCursor::COUNT> systemCursors{};
    };
    
} // namespace CE
