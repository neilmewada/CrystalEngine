#include "FusionCore.h"

#define FOCUS_LOGGING 0

namespace CE
{

    FRootContext::FRootContext()
    {

    }

    FRootContext::~FRootContext()
    {
        
    }

    void FRootContext::TickInput()
    {
        ZoneScoped;

	    Super::TickInput();

        for (int i = childContexts.GetSize() - 1; i >= 0; --i)
        {
			Ref<FFusionContext> childContext = childContexts[i];
			if (!childContext->IsOfType<FNativeContext>())
				continue;

			TickNativeContextInput(CastTo<FNativeContext>(childContext));
        }
    }

	void FRootContext::TickNativeContextInput(Ref<FNativeContext> nativeContext)
	{
		ZoneScoped;

		// Root context should only have one native context as its child.
		// The child native context can then have multiple children native contexts as different windows.

		if (nativeContext == nullptr || !IsRootContext())
			return;

		PlatformWindow* platformWindow = nativeContext->platformWindow;
		Ref<FWidget> owningWidget = nativeContext->owningWidget;

		if (!owningWidget || FusionApplication::Get()->isExposed || !platformWindow->IsShown())
		{
			return;
		}

		// - Event Handling -

		Vec2 screenMousePos = InputManager::GetGlobalMousePosition().ToVec2();

		Vec2 mousePos = nativeContext->ScreenToGlobalSpacePosition(screenMousePos);
		Vec2 mouseDelta = InputManager::GetMouseDelta().ToVec2();
		Vec2 wheelDelta = InputManager::GetMouseWheelDelta();

		if (prevMousePos.GetSqrMagnitude() == 0)
			prevMousePos = mousePos;
		if (prevScreenMousePos.GetSqrMagnitude() == 0)
			prevScreenMousePos = screenMousePos;

		MouseButtonMask curButtonMask = MouseButtonMask::None;
		if (InputManager::IsMouseButtonHeld(MouseButton::Left))
		{
			curButtonMask |= MouseButtonMask::Left;
		}
		if (InputManager::IsMouseButtonHeld(MouseButton::Right))
		{
			curButtonMask |= MouseButtonMask::Right;
		}
		if (InputManager::IsMouseButtonHeld(MouseButton::Middle))
		{
			curButtonMask |= MouseButtonMask::Middle;
		}
		if (InputManager::IsMouseButtonHeld(MouseButton::Button4))
		{
			curButtonMask |= MouseButtonMask::Button4;
		}
		if (InputManager::IsMouseButtonHeld(MouseButton::Button5))
		{
			curButtonMask |= MouseButtonMask::Button5;
		}

		Enum* keyCodeEnum = GetStaticEnum<KeyCode>();
		Enum* keyModifierEnum = GetStaticEnum<KeyModifier>();

		keyModifierStates = KeyModifier::None;
		keyPressStates.Reset();

		for (int i = 0; i < keyModifierEnum->GetConstantsCount(); ++i)
		{
			if (InputManager::TestModifiers((KeyModifier)keyModifierEnum->GetConstant(i)->GetValue()))
			{
				keyModifierStates |= (KeyModifier)keyModifierEnum->GetConstant(i)->GetValue();
			}
		}

		Ref<FWidget> hoveredWidget = nativeContext->HitTest(mousePos);

		Ref<FWidget> dropTarget = nullptr;

		if (draggedWidget != nullptr)
		{
			dropTarget = nativeContext->HitTest(mousePos, false);

			if (dropTarget)
			{
				dropTarget->Focus();
			}
		}

		while (dropTarget != nullptr && !dropTarget->SupportsDropTarget())
		{
			dropTarget = dropTarget->GetParent();
		}

		if (dropTarget && !dropTarget->SupportsDropTarget())
		{
			dropTarget = nullptr;
		}

		if (!hoveredWidgetStack.IsEmpty() && hoveredWidgetStack.Top() != hoveredWidget &&
			(hoveredWidget == nullptr || !hoveredWidget->ParentExistsRecursive(hoveredWidgetStack.Top().Get())))
		{
			FMouseEvent event{};
			event.type = FEventType::MouseLeave;
			event.mousePosition = mousePos;
			event.prevMousePosition = prevMousePos;
			event.buttons = curButtonMask;
			event.direction = FEventDirection::BottomToTop;
			event.isInside = true;
			event.keyModifiers = keyModifierStates;

			while (hoveredWidgetStack.NotEmpty() && (hoveredWidget == nullptr || (hoveredWidget != nullptr && !hoveredWidget->ParentExistsRecursive(hoveredWidgetStack.Top().Get()))))
			{
				event.sender = hoveredWidgetStack.Top().Get();
				event.Reset();

				if (event.sender.IsValid() && event.sender->SupportsMouseEvents() && event.sender->GetContext() != nullptr)
				{
					if (event.sender->GetContext() != nativeContext)
					{
						event.mousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
						event.prevMousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
					}

					event.sender->HandleEvent(&event);
				}

				hoveredWidgetStack.Pop();
			}
		}

		if (hoveredWidget != nullptr &&
			(hoveredWidgetStack.IsEmpty() || hoveredWidgetStack.Top() != hoveredWidget) &&
			(hoveredWidgetStack.IsEmpty() || !hoveredWidgetStack.Top()->ParentExistsRecursive(hoveredWidget.Get())))
		{
			FMouseEvent event{};
			event.type = FEventType::MouseEnter;
			event.mousePosition = mousePos;
			event.prevMousePosition = prevMousePos;
			event.buttons = curButtonMask;
			event.direction = FEventDirection::BottomToTop;
			event.isInside = true;
			event.keyModifiers = keyModifierStates;

			int idx = hoveredWidgetStack.GetSize();
			FWidget* basePrevWidget = nullptr;
			if (hoveredWidgetStack.NotEmpty())
				basePrevWidget = hoveredWidgetStack.Top().Get();

			auto widget = hoveredWidget;

			while ((hoveredWidgetStack.IsEmpty() || widget != basePrevWidget) && widget != nullptr)
			{
				hoveredWidgetStack.InsertAt(idx, widget);
				widget = widget->parent.Get();
			}

			for (int i = idx; i < hoveredWidgetStack.GetSize(); ++i)
			{
				event.sender = hoveredWidgetStack[i].Get();
				event.Reset();

				if (event.sender->GetContext() != nativeContext)
				{
					event.mousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
					event.prevMousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
				}

				event.sender->HandleEvent(&event);
			}
		}

		// - Mouse Wheel Events -
        constexpr auto epsilon = std::numeric_limits<float>::epsilon();
        
		if (abs(wheelDelta.x) >= epsilon || abs(wheelDelta.y) >= epsilon)
		{
			FMouseEvent mouseEvent{};
			mouseEvent.type = FEventType::MouseWheel;
			mouseEvent.buttons = curButtonMask;
			mouseEvent.mousePosition = mousePos;
			mouseEvent.prevMousePosition = prevMousePos;
			mouseEvent.direction = FEventDirection::BottomToTop;
			mouseEvent.wheelDelta = wheelDelta;
			mouseEvent.keyModifiers = keyModifierStates;

			if (hoveredWidgetStack.NotEmpty())
			{
				FWidget* sender = hoveredWidgetStack.Top().Get();

				while (sender != nullptr && !sender->CapturesMouseWheel())
				{
					sender = sender->parent.Get();
				}

				if (sender)
				{
					mouseEvent.sender = sender;

					if (mouseEvent.sender->GetContext() != nativeContext)
					{
						mouseEvent.mousePosition = mouseEvent.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
						mouseEvent.prevMousePosition = mouseEvent.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
					}

					sender->HandleEvent(&mouseEvent);
				}
			}
		}

		// - Mouse Move -

        if (abs(mouseDelta.x) >= epsilon || abs(mouseDelta.y) >= epsilon)
		{
			FMouseEvent mouseEvent{};
			mouseEvent.type = FEventType::MouseMove;
			mouseEvent.buttons = curButtonMask;
			mouseEvent.mousePosition = mousePos;
			mouseEvent.prevMousePosition = prevMousePos;
			mouseEvent.direction = FEventDirection::BottomToTop;
			mouseEvent.wheelDelta = wheelDelta;
			mouseEvent.keyModifiers = keyModifierStates;

			if (hoveredWidgetStack.NotEmpty())
			{
				mouseEvent.sender = hoveredWidgetStack.Top().Get();

				if (mouseEvent.sender->GetContext() != nullptr && mouseEvent.sender->GetContext() != nativeContext)
				{
					mouseEvent.mousePosition = mouseEvent.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
					mouseEvent.prevMousePosition = mouseEvent.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
				}

				hoveredWidgetStack.Top()->HandleEvent(&mouseEvent);
			}

			if (draggedWidget != nullptr && curButtonMask != MouseButtonMask::None)
			{
				FDragEvent dragEvent{};
				dragEvent.type = FEventType::DragMove;
				dragEvent.buttons = curButtonMask;
				dragEvent.mousePosition = mousePos;
				dragEvent.prevMousePosition = prevMousePos;
				dragEvent.direction = FEventDirection::BottomToTop;
				dragEvent.wheelDelta = wheelDelta;
				dragEvent.isInside = true;
				dragEvent.keyModifiers = keyModifierStates;
				dragEvent.dropTarget = dropTarget;

				dragEvent.sender = draggedWidget.Get();
				if (hoveredWidgetStack.NotEmpty())
					dragEvent.sender = hoveredWidgetStack.Top().Get();
				dragEvent.draggedWidget = draggedWidget.Get();

				if (Ref<FFusionContext> draggedWidgetContext = draggedWidget->GetContext())
				{
					if (draggedWidgetContext != nativeContext)
					{
						dragEvent.mousePosition = draggedWidgetContext->ScreenToGlobalSpacePosition(screenMousePos);
						dragEvent.prevMousePosition = draggedWidgetContext->ScreenToGlobalSpacePosition(prevScreenMousePos);
					}
				}

				bool isValid = draggedWidget.IsValid();
				draggedWidget->HandleEvent(&dragEvent);

				// Cancel dragging if any of the FDragEvent's are not consumed by the dragged widget
				if (!dragEvent.isConsumed)
				{
					draggedWidget = nullptr;
				}
				else
				{
					draggedWidget = dragEvent.draggedWidget;
				}
			}
		}

		if (InputManager::IsMouseButtonDown(MouseButton::Left))
		{
			Array<FPopup*> popupsToClose{};

			for (int i = localPopupStack.GetSize() - 1; i >= 0; --i)
			{
				if (!localPopupStack[i]->AutoClose())
					continue;

				if (hoveredWidget == nullptr || !hoveredWidget->ParentExistsRecursive(localPopupStack[i].Get()))
				{
					popupsToClose.Add(localPopupStack[i].Get());
				}
			}

			for (FPopup* popup : popupsToClose)
			{
				ClosePopup(popup);
			}
		}

		if (hoveredWidget != prevHoveredWidget)
		{
			prevHoveredWidget = hoveredWidget;
		}

		// - Focus Events -

		if (owningWidget && IsFocused() && !owningWidget->IsFocused())
		{
			FFocusEvent focusEvent{};
			focusEvent.gotFocus = true;
			focusEvent.type = FEventType::FocusChanged;
			focusEvent.sender = owningWidget.Get();
			focusEvent.focusedWidget = owningWidget.Get();
			focusEvent.direction = FEventDirection::BottomToTop;

#if FOCUS_LOGGING
			CE_LOG(Info, All, "Focus GOT 1: {}", owningWidget->GetName());
#endif

			owningWidget->HandleEvent(&focusEvent);
		}
		else if (owningWidget && !IsFocused() && owningWidget->IsFocused())
		{
			FFocusEvent focusEvent{};
			focusEvent.gotFocus = false;
			focusEvent.type = FEventType::FocusChanged;
			focusEvent.sender = owningWidget.Get();
			focusEvent.focusedWidget = nullptr;
			focusEvent.direction = FEventDirection::TopToBottom;

#if FOCUS_LOGGING
			CE_LOG(Info, All, "Focus LOST 1: {}", owningWidget->GetName());
#endif

			owningWidget->HandleEvent(&focusEvent);

			curFocusWidget = nullptr;
			widgetToFocus = nullptr;
		}

		if (curFocusWidget != nullptr && !curFocusWidget->IsFocused() && curFocusWidget == widgetToFocus)
		{
			curFocusWidget = nullptr;
			widgetToFocus = nullptr;
		}

		if (widgetToFocus != curFocusWidget)
		{
			if (curFocusWidget != nullptr && (widgetToFocus == nullptr || !widgetToFocus->FocusParentExistsRecursive(curFocusWidget.Get())))
			{
				Ref<FWidget> base = curFocusWidget.Lock();

				while (base != nullptr)
				{
					if (base->parent != nullptr && widgetToFocus.IsValid() && widgetToFocus->FocusParentExistsRecursive(base->parent.Get()))
					{
						break;
					}
					base = base->parent.Get();
				}

				if (base != nullptr)
				{
					FFocusEvent focusEvent{};
					focusEvent.type = FEventType::FocusChanged;
					focusEvent.sender = base.Get();
					focusEvent.focusedWidget = widgetToFocus.Get();
					focusEvent.gotFocus = false;
					focusEvent.direction = FEventDirection::TopToBottom;

#if FOCUS_LOGGING
					CE_LOG(Info, All, "Focus LOST 2: {}", base->GetName());
#endif

					base->HandleEvent(&focusEvent);
				}
				else
				{
					FFocusEvent focusEvent{};
					focusEvent.type = FEventType::FocusChanged;
					focusEvent.sender = curFocusWidget.Get();
					focusEvent.focusedWidget = widgetToFocus.Get();
					focusEvent.gotFocus = false;
					focusEvent.direction = FEventDirection::BottomToTop;

#if FOCUS_LOGGING
					CE_LOG(Info, All, "Focus LOST 3: {}", curFocusWidget->GetName());
#endif

					curFocusWidget->HandleEvent(&focusEvent);
				}
			}

			if (widgetToFocus != nullptr)
			{
				FFocusEvent focusEvent{};
				focusEvent.type = FEventType::FocusChanged;
				focusEvent.sender = focusEvent.focusedWidget = widgetToFocus.Get();
				focusEvent.gotFocus = true;
				focusEvent.direction = FEventDirection::BottomToTop;

#if FOCUS_LOGGING
				CE_LOG(Info, All, "Focus GOT 2: {}", widgetToFocus->GetName());
#endif

				widgetToFocus->HandleEvent(&focusEvent);
			}

			curFocusWidget = widgetToFocus;
		}

		// - Mouse Press & Release -

		Enum* mouseButtonEnum = GetStaticEnum<MouseButton>();
		for (int i = 0; i < mouseButtonEnum->GetConstantsCount(); ++i)
		{
			MouseButton mouseButton = (MouseButton)mouseButtonEnum->GetConstant(i)->GetValue();
			if (InputManager::IsMouseButtonDown(mouseButton))
			{
				FMouseEvent event{};
				event.type = FEventType::MousePress;
				event.mousePosition = mousePos;
				event.prevMousePosition = prevMousePos;
				event.buttons = (MouseButtonMask)BIT((int)mouseButton);
				event.direction = FEventDirection::BottomToTop;
				event.keyModifiers = keyModifierStates;
				event.isDoubleClick = InputManager::GetMouseButtonClicks(mouseButton) == 2;

				if (hoveredWidgetStack.NotEmpty())
				{
					event.sender = hoveredWidgetStack.Top().Get();
					widgetToFocus = event.sender;

					while (event.sender != nullptr && !event.sender->SupportsMouseEvents())
					{
						event.sender = event.sender->parent.Get();
					}

					FWidget* dragEventWidget = hoveredWidgetStack.Top().Get();

					while (dragEventWidget != nullptr)
					{
						if (dragEventWidget->SupportsDragEvents())
						{
							FDragEvent dragEvent{};
							dragEvent.type = FEventType::DragBegin;
							dragEvent.mousePosition = mousePos;
							dragEvent.prevMousePosition = prevMousePos;
							dragEvent.buttons = (MouseButtonMask)BIT((int)mouseButton);
							dragEvent.direction = FEventDirection::BottomToTop;
							dragEvent.keyModifiers = keyModifierStates;
							dragEvent.isDoubleClick = InputManager::GetMouseButtonClicks(mouseButton) == 2;

							dragEvent.sender = dragEventWidget;
							dragEvent.draggedWidget = dragEventWidget;

							if (dragEvent.sender->GetContext() != nativeContext)
							{
								dragEvent.mousePosition = dragEvent.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
								dragEvent.prevMousePosition = dragEvent.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
							}

							dragEvent.sender->HandleEvent(&dragEvent);

							if (dragEvent.isConsumed)
							{
								draggedWidget = dragEvent.draggedWidget;

								event.consumer = dragEventWidget;
								event.isConsumed = true;
							}

							break;
						}

						dragEventWidget = dragEventWidget->parent.Get();
					}

					if (event.sender != nullptr)
					{
						widgetsPressedPerMouseButton[i] = event.sender;

						if (event.sender->GetContext() != nativeContext)
						{
							event.mousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
							event.prevMousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
						}

						event.sender->HandleEvent(&event);
					}
				}
			}

			if (InputManager::IsMouseButtonUp(mouseButton))
			{
				FMouseEvent event{};
				event.type = FEventType::MouseRelease;
				event.mousePosition = mousePos;
				event.prevMousePosition = prevMousePos;
				event.buttons = (MouseButtonMask)BIT((int)mouseButton);
				event.direction = FEventDirection::BottomToTop;
				event.isInside = true;
				event.keyModifiers = keyModifierStates;

				if (hoveredWidgetStack.NotEmpty())
				{
					event.sender = hoveredWidgetStack.Top().Get();
					while (event.sender != nullptr && !event.sender->SupportsMouseEvents())
					{
						event.sender = event.sender->parent.Get();
					}

					if (event.sender != nullptr)
					{
						if (event.sender->GetContext() != nativeContext)
						{
							event.mousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
							event.prevMousePosition = event.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
						}

						event.sender->HandleEvent(&event);
					}
				}

				if (widgetsPressedPerMouseButton[i] != nullptr && widgetsPressedPerMouseButton[i] != event.sender)
				{
					event.Reset();
					event.isInside = false;

					if (widgetsPressedPerMouseButton[i]->GetContext() != nativeContext)
					{
						event.mousePosition = widgetsPressedPerMouseButton[i]->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
						event.prevMousePosition = widgetsPressedPerMouseButton[i]->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
					}

					widgetsPressedPerMouseButton[i]->HandleEvent(&event);
				}

				if (mouseButton == MouseButton::Left && draggedWidget != nullptr)
				{
					FDragEvent dragEvent{};
					dragEvent.type = FEventType::DragEnd;
					dragEvent.mousePosition = mousePos;
					dragEvent.prevMousePosition = prevMousePos;
					dragEvent.buttons = (MouseButtonMask)BIT((int)mouseButton);
					dragEvent.keyModifiers = keyModifierStates;
					dragEvent.isInside = false;
					dragEvent.dropTarget = dropTarget;

					if (hoveredWidgetStack.NotEmpty())
					{
						for (int j = hoveredWidgetStack.GetSize() - 1; j >= 0; --j)
						{
							if (hoveredWidgetStack[j] == draggedWidget)
							{
								dragEvent.isInside = true;
								break;
							}
						}
					}

					dragEvent.sender = draggedWidget.Get();
					dragEvent.draggedWidget = draggedWidget.Get();

					if (dragEvent.sender->GetContext() != nativeContext)
					{
						dragEvent.mousePosition = dragEvent.sender->GetContext()->ScreenToGlobalSpacePosition(screenMousePos);
						dragEvent.prevMousePosition = dragEvent.sender->GetContext()->ScreenToGlobalSpacePosition(prevScreenMousePos);
					}

					dragEvent.sender->HandleEvent(&dragEvent);

					draggedWidget = nullptr;
				}

				widgetsPressedPerMouseButton[i] = nullptr;
			}
		}

		// - Keyboard Events -

		Ref<FWidget> keyEventWidget = curFocusWidget.Lock();
		while (keyEventWidget != nullptr && !keyEventWidget->SupportsKeyboardEvents())
		{
			keyEventWidget = keyEventWidget->parent.Get();
		}

		if (keyEventWidget != nullptr)
		{
			for (int i = 0; i < keyCodeEnum->GetConstantsCount(); ++i)
			{
				KeyCode keyCode = (KeyCode)keyCodeEnum->GetConstant(i)->GetValue();

				bool isDown = InputManager::IsKeyDown(keyCode);
				bool isUp = InputManager::IsKeyUp(keyCode);

				bool isHeldDelayed = InputManager::IsKeyHeldDelayed(keyCode);

				FKeyEvent keyEvent{};
				keyEvent.modifiers = keyModifierStates;
				keyEvent.key = keyCode;
				keyEvent.sender = keyEventWidget.Get();

				if (isDown && !keyPressStates.Test(i))
				{
					keyEvent.type = FEventType::KeyPress;

					keyPressStates.Set(i, true);

					keyEventWidget->HandleEvent(&keyEvent);
				}
				else if (isUp && keyPressStates.Test(i))
				{
					keyEvent.type = FEventType::KeyRelease;

					keyPressStates.Set(i, false);

					keyEventWidget->HandleEvent(&keyEvent);
				}

				if (isHeldDelayed)
				{
					keyEvent.type = FEventType::KeyHeld;

					keyEventWidget->HandleEvent(&keyEvent);
				}
			}
		}

		prevMousePos = mousePos;
		prevScreenMousePos = screenMousePos;
	}
    
} // namespace CE

