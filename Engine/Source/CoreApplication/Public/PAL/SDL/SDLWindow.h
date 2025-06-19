#pragma once

struct SDL_Window;

namespace CE
{

    class COREAPPLICATION_API SDLPlatformWindow : public PlatformWindow
    {
    public:

        ~SDLPlatformWindow();

        virtual void* GetUnderlyingHandle() override;
		virtual WindowHandle GetOSNativeHandle() override;
        virtual String GetTitle() override;

        virtual int GetZOrder() override;

        SDL_Window* GetSdlHandle() const { return handle; }

        u32 GetWindowDpi() override;

        void Show() override;

        void Hide() override;

		bool IsFocused() override;

        Vec2i GetWindowPosition() override;
        void SetWindowPosition(Vec2i position) override;

        virtual void GetWindowSize(u32* outWidth, u32* outHeight) override;
        virtual Vec2i GetWindowSize() override;
        virtual void SetWindowSize(Vec2i newSize) override;
        virtual void SetMinimumSize(Vec2i minSize) override;

        virtual PlatformWindowFlags GetInitialFlags() override { return initialFlags; }

        virtual void SetOpacity(f32 opacity) override;
        virtual void SetAlwaysOnTop(bool alwaysOnTop) override;

        virtual void GetDrawableWindowSize(u32* outWidth, u32* outHeight) override;
        virtual Vec2i GetDrawableWindowSize() override;

		virtual void SetResizable(bool resizable) override;
        virtual void SetBorderless(bool borderless) override;
        virtual void RaiseWindow() override;

        virtual bool IsBorderless() override;
        virtual bool IsMinimized() override;
        virtual bool IsMaximized() override;
        virtual bool IsFullscreen() override;
        virtual bool IsShown() override;
        virtual bool IsHidden() override;
        virtual bool IsAlwaysOnTop() override;
        virtual bool IsResizable() override;

        virtual void Minimize() override;
        virtual void Restore() override;
        virtual void Maximize() override;

        VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;

        u64 GetWindowId() override;

    protected:

        friend class SDLApplication;
        
        SDLPlatformWindow(const String& title, u32 width, u32 height, bool maximised, bool fullscreen, bool resizable = true, bool isHidden = false, int displayIndex = -1);

        SDLPlatformWindow(const String& title, u32 width, u32 height, const PlatformWindowInfo& info);

        SDL_Window* handle = nullptr;

        PlatformWindowFlags initialFlags{};

    private:

    };

    
} // namespace CE
