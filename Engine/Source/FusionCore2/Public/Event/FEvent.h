#pragma once

namespace CE
{
    class FWidget;

    ENUM()
    enum class FEventType
    {
	    None = 0,

        MouseEnter,
        MouseLeave,
        MouseMove,
        MouseWheel,

        MouseButtonDown,
        MouseButtonUp,
        
        KeyDown,
        KeyUp,

        FocusChanged,
    };
    ENUM_CLASS(FEventType);

    struct FEventReply
    {
    private:

        bool handled = false;
        bool captureMouse = false;
        bool releaseMouse = false;
        bool focusSelf = false;
        bool hideCursor = false;
        bool showCursor = false;

    public:

        // --- Constructors ---

        static FEventReply Handled() { FEventReply r; r.handled = true; return r; }
        static FEventReply Unhandled() { return {}; }

        // --- Intent Builders ---

        FEventReply& CaptureMouse() { captureMouse = true;  return *this; }
        FEventReply& ReleaseMouse() { releaseMouse = true;  return *this; }
        FEventReply& FocusSelf() { focusSelf = true;     return *this; }
        FEventReply& HideCursor() { hideCursor = true;    return *this; }
        FEventReply& ShowCursor() { showCursor = true;    return *this; }

        // --- Accessors (for FEventService::ProcessReply) ---

        bool IsHandled()          const { return handled; }
        bool ShouldCaptureMouse() const { return captureMouse; }
        bool ShouldReleaseMouse() const { return releaseMouse; }
        bool ShouldFocusSelf()    const { return focusSelf; }
        bool ShouldHideCursor()   const { return hideCursor; }
        bool ShouldShowCursor()   const { return showCursor; }
    };

    STRUCT()
    struct FUSIONCORE_API FEvent
    {
        CE_STRUCT(FEvent)
    public:

        virtual ~FEvent() = default;

        FIELD()
        FEventType eventType = FEventType::None;

        FIELD()
        Ref<FWidget> sender = nullptr;

        bool IsMouseEvent() const
        {
	        switch (eventType)
	        {
            case FEventType::MouseButtonDown:
            case FEventType::MouseButtonUp:
            case FEventType::MouseEnter:
            case FEventType::MouseLeave:
            case FEventType::MouseMove:
            case FEventType::MouseWheel:
                return true;
	        default:
                return false;
	        }
        }

        bool IsKeyEvent() const
        {
	        switch (eventType)
	        {
            case FEventType::KeyDown:
            case FEventType::KeyUp:
                return true;
            default:
                return false;
	        }
        }

        bool IsFocusEvent() const
        {
            return eventType == FEventType::FocusChanged;
        }
    };

    STRUCT()
    struct FUSIONCORE_API FMouseEvent : FEvent
    {
        CE_STRUCT(FMouseEvent, FEvent)
    public:

        //! @brief Mouse position in global space (i.e. surface space)
        FIELD()
        Vec2 mousePosition;

        FIELD()
        Vec2 prevMousePosition;

        FIELD()
        Vec2 wheelDelta;

        FIELD()
        MouseButtonMask buttons = MouseButtonMask::None;

        FIELD()
        b8 isInside = false;

        FIELD()
        b8 isDoubleClick = false;

        FIELD()
        KeyModifier keyModifiers = KeyModifier::None;

        bool IsLeftButton() const { return EnumHasFlag(buttons, MouseButtonMask::Left); }
        bool IsRightButton() const { return EnumHasFlag(buttons, MouseButtonMask::Right); }
        bool IsMiddleButton() const { return EnumHasFlag(buttons, MouseButtonMask::Middle); }

        bool IsMultiSelectionModifier() const
        {
#if PLATFORM_MAC
            return EnumHasAnyFlags(keyModifiers, KeyModifier::Gui | KeyModifier::Shift);
#else
            return EnumHasAnyFlags(keyModifiers, KeyModifier::Ctrl | KeyModifier::Shift);
#endif
        }

        bool IsCtrlMultiSelectionModifier() const
        {
#if PLATFORM_MAC
            return EnumHasAnyFlags(keyModifiers, KeyModifier::Gui);
#else
            return EnumHasAnyFlags(keyModifiers, KeyModifier::Ctrl);
#endif
        }

        bool IsShiftMultiSelectionModifier() const
        {
            return EnumHasAnyFlags(keyModifiers, KeyModifier::Shift);
        }
    };

    STRUCT()
    struct FUSIONCORE_API FKeyEvent : FEvent
    {
        CE_STRUCT(FKeyEvent, FEvent)
    public:

        FKeyEvent() {}

        FIELD()
        KeyCode key = KeyCode::Unknown;

        FIELD()
        KeyModifier modifiers = KeyModifier::None;

    };

    STRUCT()
    struct FUSIONCORE_API FFocusEvent : FEvent
    {
        CE_STRUCT(FFocusEvent, FEvent)
    public:

        FFocusEvent() {}

        bool GotFocus() const { return gotFocus; }
        bool LostFocus() const { return !gotFocus; }

        FIELD()
        bool gotFocus = false;

        FIELD()
        Ref<FWidget> focusedWidget = nullptr;

    };

    
} // namespace CE

#include "FEvent.rtti.h"