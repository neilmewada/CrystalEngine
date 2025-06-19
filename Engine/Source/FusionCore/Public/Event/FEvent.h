#pragma once


namespace CE
{
    class FWidget;
    class FFusionContext;
    class FNativeContext;

    ENUM()
    enum class FEventType
    {
	    None = 0,

        NativeWindowExposed,
        NativeWindowMoved,
        NativeWindowResized,

        MouseEnter,
        MouseMove,
        MouseLeave,
        MousePress,
        MouseRelease,
        MouseWheel,

        DragBegin,
        DragMove,
        DragEnd,

        FocusChanged,

        KeyPress,
        KeyHeld,
        KeyRelease,

        HierarchyDetached
    };
    ENUM_CLASS(FEventType);

    ENUM()
    enum class FEventDirection
    {
	    BottomToTop = 0,
        TopToBottom
    };
    ENUM_CLASS(FEventDirection);

    STRUCT()
    struct FUSIONCORE_API FEvent
    {
        CE_STRUCT(FEvent)
    public:

        virtual ~FEvent() {}

        FIELD()
        FEventType type = FEventType::None;

        FIELD()
        FEventDirection direction = FEventDirection::BottomToTop;

        FIELD()
        Ref<FWidget> sender = nullptr;

        FIELD()
        Ref<FWidget> consumer = nullptr;

        bool isConsumed : 1 = false;

        bool stopPropagation : 1 = false;

        void Reset()
        {
            consumer = nullptr;
            isConsumed = stopPropagation = false;
        }

        void Consume(Ref<FWidget> consumer)
        {
            if (isConsumed)
                return;

            this->consumer = consumer;
            isConsumed = true;
        }

        void ConsumeAndStopPropagation(Ref<FWidget> consumer)
        {
            Consume(consumer);
            stopPropagation = true;
        }

        bool IsMouseEvent() const
        {
	        switch (type)
	        {
	        case FEventType::MouseEnter:
	        case FEventType::MouseMove:
            case FEventType::MouseLeave:
            case FEventType::MousePress:
            case FEventType::MouseRelease:
            case FEventType::MouseWheel:
            case FEventType::DragBegin:
            case FEventType::DragMove:
            case FEventType::DragEnd:
                return true;
            default:
                return false;
	        }
        }

        bool IsDragEvent() const
        {
	        switch (type)
	        {
            case FEventType::DragBegin:
            case FEventType::DragMove:
            case FEventType::DragEnd:
                return true;
            default:
                return false;
	        }
        }

        bool IsKeyEvent() const
        {
	        switch (type)
	        {
	        case FEventType::KeyPress:
	        case FEventType::KeyHeld:
	        case FEventType::KeyRelease:
                return true;
            default:
                return false;
	        }
        }
    };

    STRUCT()
    struct FUSIONCORE_API FHierarchyEvent : FEvent
    {
        CE_STRUCT(FHierarchyEvent, FEvent)
    public:

        FIELD()
        Ref<FWidget> parentWidget = nullptr; // The widget that the topMostWidget was attached to or detached from.

        FIELD()
        Ref<FWidget> topMostWidget = nullptr; // The top-most widget that was attached/detached.

    };

    STRUCT()
    struct FUSIONCORE_API FMouseEvent : FEvent
    {
        CE_STRUCT(FMouseEvent, FEvent)
    public:

        //! @brief Mouse position in global space (i.e. context space)
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
    struct FUSIONCORE_API FDragEvent : FMouseEvent
    {
        CE_STRUCT(FDragEvent, FMouseEvent)
    public:

        FIELD()
        Ref<FWidget> draggedWidget = nullptr;

		FIELD()
		Ref<FWidget> dropTarget = nullptr;

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
    struct FUSIONCORE_API FNativeEvent : FEvent
    {
        CE_STRUCT(FNativeEvent, FEvent)
    public:

        FNativeEvent() {}

        FIELD()
        Ref<FNativeContext> nativeContext = nullptr;


    };
    
} // namespace CE

#include "FEvent.rtti.h"