#pragma once

struct VkInstance_T;
typedef VkInstance_T* VkInstance;

struct VkSurfaceKHR_T;
typedef VkSurfaceKHR_T* VkSurfaceKHR;

namespace CE
{
	typedef SIZE_T WindowHandle;
	class PlatformWindow;

    typedef Delegate<bool(PlatformWindow* window, Vec2 position)> WindowDragHitTestDelegate;
    
    class COREAPPLICATION_API PlatformWindow
    {
    protected:
        PlatformWindow();

        virtual ~PlatformWindow();

    public:

        virtual void* GetUnderlyingHandle() = 0;

		virtual WindowHandle GetOSNativeHandle() = 0;

        virtual u32 GetWindowDpi() = 0;

        virtual String GetTitle() = 0;

        virtual int GetZOrder() = 0;

        virtual void Show() = 0;
        virtual void Hide() = 0;

		virtual bool IsFocused() = 0;

        virtual Vec2i GetWindowPosition() = 0;
        virtual void SetWindowPosition(Vec2i position) = 0;

        virtual void GetWindowSize(u32* outWidth, u32* outHeight) = 0;
        virtual Vec2i GetWindowSize() = 0;
        virtual void SetWindowSize(Vec2i newSize) = 0;

        virtual PlatformWindowFlags GetInitialFlags() = 0;

        virtual void GetDrawableWindowSize(u32* outWidth, u32* outHeight) = 0;
        virtual Vec2i GetDrawableWindowSize() = 0;

        virtual VkSurfaceKHR CreateVulkanSurface(VkInstance instance) = 0;

        virtual void SetMinimumSize(Vec2i minSize) = 0;
        virtual void SetOpacity(f32 opacity) = 0;
        virtual void SetAlwaysOnTop(bool alwaysOnTop) = 0;
		virtual void SetResizable(bool resizable) = 0;
        virtual void SetBorderless(bool borderless) = 0;
        virtual void RaiseWindow() = 0;

        virtual bool IsBorderless() = 0;
        virtual bool IsMinimized() = 0;
        virtual bool IsMaximized() = 0;
        virtual bool IsFullscreen() = 0;
        virtual bool IsShown() = 0;
        virtual bool IsHidden() = 0;
        virtual bool IsAlwaysOnTop() = 0;
        virtual bool IsResizable() = 0;

        virtual void Minimize() = 0;
        virtual void Restore() = 0;
        virtual void Maximize() = 0;

        virtual u64 GetWindowId() = 0;

        inline bool IsMainWindow() const { return isMainWindow; }

        void SetHitTestDelegate(const WindowDragHitTestDelegate& delegate)
        {
            this->dragHitTest = delegate;
        }

    coreapplication_internal:

        WindowDragHitTestDelegate dragHitTest{};

        bool isMainWindow = false;

		friend class PlatformApplication;
    };

} // namespace CE
