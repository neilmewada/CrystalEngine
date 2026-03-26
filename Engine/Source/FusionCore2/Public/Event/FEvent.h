#pragma once

namespace CE
{
    class FWidget;
    DECLARE_SCRIPT_EVENT(FVoidEvent);

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
    public:

        enum class MouseCaptureOp { None, Capture, Release };
        enum class InputLockOp { None, Lock, Unlock };
        enum class CursorOp { None, Hide, Show };

        // -- Constructors --

        static FEventReply Handled() { FEventReply r; r.handled = true; return r; }
        static FEventReply Unhandled() { return {}; }

        // -- Intent builders --

        // Soft mouse capture: mouse events keep routing here even when cursor leaves bounds.
        // Does NOT affect keyboard. Does NOT hide cursor.
        FEventReply& CaptureMouse() { mouseCaptureOp = MouseCaptureOp::Capture;  return *this; }
        FEventReply& ReleaseMouse() { mouseCaptureOp = MouseCaptureOp::Release;  return *this; }

        // Full input lock: ALL mouse + keyboard route here. Hides and locks cursor.
        // Use for 3D viewport / game input capture.
        FEventReply& LockInput() { inputLockOp = InputLockOp::Lock;           return *this; }
        FEventReply& UnlockInput() { inputLockOp = InputLockOp::Unlock;         return *this; }

        // Keyboard focus
        FEventReply& FocusSelf() { focusSelf = true;                          return *this; }

        // Cursor visibility — independent of capture (e.g. custom cursor hide without locking)
        //FEventReply& HideCursor() { cursorOp = CursorOp::Hide;                 return *this; }
        //FEventReply& ShowCursor() { cursorOp = CursorOp::Show;                 return *this; }

        // -- Accessors for FEventService::ProcessReply --

        bool              IsHandled()           const { return handled; }
        bool              ShouldFocusSelf()     const { return focusSelf; }
        MouseCaptureOp    GetMouseCaptureOp()   const { return mouseCaptureOp; }
        InputLockOp       GetInputLockOp()      const { return inputLockOp; }
        CursorOp          GetCursorOp()         const { return cursorOp; }

    private:
        bool           handled = false;
        bool           focusSelf = false;
        MouseCaptureOp mouseCaptureOp = MouseCaptureOp::None;
        InputLockOp    inputLockOp = InputLockOp::None;
        CursorOp       cursorOp = CursorOp::None;
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

        //! @brief Previous mouse position in global space (i.e. surface space)
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