#include "FusionCore.h"

namespace CE
{

	FFusionContext::FFusionContext()
	{

	}

	FFusionContext::~FFusionContext()
	{

	}

	bool FFusionContext::IsNativeContext() const
	{
		return IsOfType<FNativeContext>();
	}

	void FFusionContext::Tick()
	{
		ZoneScoped;

		DoLayout();

		DoPaint();

		// TODO: Better integration of child contexts and native popups

		TickInput();

		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->Tick();
		}
	}

	void FFusionContext::DoLayout()
	{
		ZoneScoped;

		if (owningWidget && layoutDirty)
		{
			layoutDirty = false;
			dirty = true;

			owningWidget->CalculateIntrinsicSize();

			Vec2 availSize = GetAvailableSize();

			owningWidget->computedPosition = Vec2();
			owningWidget->computedSize = availSize;
			
			owningWidget->PlaceSubWidgets();

			for (const auto& popup : localPopupStack)
			{
				popup->CalculateIntrinsicSize();

				Vec2 size = popup->initialSize;
				if (size.x <= 0)
				{
					size.x = popup->intrinsicSize.x;
				}
				if (size.y <= 0)
				{
					size.y = popup->intrinsicSize.y;
				}

				popup->computedPosition = popup->initialPos;
				popup->computedSize = size;

				popup->PlaceSubWidgets();

				if (!popup->positionFound)
				{
					Rect popupRect = Rect::FromSize(popup->computedPosition, popup->computedSize);
					if (popupRect.max.y > availSize.y) // Popup outside bottom
					{
						popup->computedPosition.y -= popup->computedSize.y + popup->controlSize.y;
					}
					if (popupRect.max.x > availSize.x)
					{
						popup->computedPosition.x -= popupRect.max.x - availSize.x * availableSizeMultiplier.x;
					}
					popup->initialPos = popup->computedPosition;

					MarkLayoutDirty();
					popup->positionFound = true;
				}
			}
		}

		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->DoLayout();
		}
	}

	void FFusionContext::DoPaint()
	{

	}

	void FFusionContext::TickInput()
	{
		ZoneScoped;

	}

	int FFusionContext::GetZOrder()
	{
		return NumericLimits<int>::Max();
	}

	FFusionContext* FFusionContext::GetRootContext() const
	{
		return FusionApplication::Get()->rootContext.Get();
	}

	void FFusionContext::SetOwningWidget(FWidget* widget)
	{
		owningWidget = widget;

		if (owningWidget)
		{
			owningWidget->SetContextRecursively(this);
		}
	}

	FStyleSet* FFusionContext::GetDefaultStyleSet()
	{
		if (!defaultStyleSet && parentContext)
		{
			return parentContext->GetDefaultStyleSet();
		}
		return defaultStyleSet.Get();
	}

	void FFusionContext::OnWidgetDestroyed(FWidget* widget)
	{
		if (hoveredWidgetStack.Exists(widget))
		{
			hoveredWidgetStack.Remove(widget);
		}

		if (!IsRootContext())
		{
			FusionApplication::Get()->GetRootContext()->OnWidgetDestroyed(widget);
		}

		if (owningWidget == widget)
		{
			owningWidget = nullptr;
		}

		if (curFocusWidget == widget)
		{
			curFocusWidget = nullptr;
		}

		if (widgetToFocus == widget)
		{
			widgetToFocus = nullptr;
		}
	}

	Vec2 FFusionContext::GetAvailableSize() const
	{
		if (IsGhosted())
		{
			return ghostedAvailableSize * availableSizeMultiplier;
		}
		return availableSize * availableSizeMultiplier;
	}

	bool FFusionContext::ParentContextExistsRecursive(Ref<FFusionContext> parent) const
	{
		if (parent == parentContext.Get())
			return true;

		if (!parentContext)
			return false;

		return parentContext->ParentContextExistsRecursive(parent);
	}

	bool FFusionContext::IsFocused() const
	{
		if (IsDefaultInstance())
			return false;

		if (parentContext == nullptr) // Root context
			return true;

		for (Ref<FFusionContext> childContext : childContexts)
		{
			if (childContext->IsFocused())
				return true;
		}

		return false;
	}

	bool FFusionContext::IsShown() const
	{
		if (parentContext == nullptr)
			return true;

		return true;
	}

	void FFusionContext::SetContextFocus()
	{

	}

	bool FFusionContext::IsRootContext() const
	{
		return IsOfType<FRootContext>();
	}

	void FFusionContext::MarkLayoutDirty()
	{
		layoutDirty = true;
		dirty = true;

		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->MarkLayoutDirty();
		}
	}

	void FFusionContext::MarkDirty()
	{
		dirty = true;

		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->MarkDirty();
		}
	}

	void FFusionContext::QueueDestroy()
	{
		if (Thread::GetCurrentThreadId() != gMainThreadId)
		{
			String::IsAlphabet('a');
		}

		if (isDestroyed)
			return;

		if (parentContext)
		{
			parentContext->RemoveChildContext(this);
		}

		if (owningWidget)
		{
			owningWidget->SetContextRecursively(nullptr);
		}

		SetOwningWidget(nullptr);

		FusionApplication::Get()->destructionQueue.Add({ this, 1 });
		isDestroyed = true;

		OnQueuedDestroy();
	}

	void FFusionContext::OnQueuedDestroy()
	{
		
	}

	void FFusionContext::AddChildContext(FFusionContext* context)
	{
		if (childContexts.Exists(context))
			return;

		context->parentContext = this;
		childContexts.Add(context);
	}

	void FFusionContext::RemoveChildContext(FFusionContext* context)
	{
		if (!childContexts.Exists(context))
			return;

		context->parentContext = nullptr;
		childContexts.Remove(context);
	}

	void FFusionContext::PushLocalPopup(FPopup* popup, Vec2 globalPosition, Vec2 size, Vec2 controlSize)
	{
		if (!popup)
			return;
		if (localPopupStack.Exists(popup))
			return;

		localPopupStack.Push(popup);
		popup->context = this;
		popup->initialPos = globalPosition;
		popup->initialSize = size;
		popup->isShown = true;
		popup->isNativePopup = false;
		popup->positionFound = false;
		popup->controlSize = controlSize;

		popup->SetContextRecursively(this);

		MarkLayoutDirty();

		GetRootContext()->SetFocusWidget(popup);

		popup->ApplyStyleRecursively();
	}

	void FFusionContext::PushNativePopup(FPopup* popup, Vec2 globalPosition, Vec2 size)
	{
		
	}

	bool FFusionContext::ClosePopup(Ref<FPopup> popup)
	{
		if (!popup)
			return true;
		if (!popup->isShown)
			return true;

		if (popup->isNativePopup)
		{
			Ref<FFusionContext> popupContext = popup->GetContext();
			if (popupContext)
			{
				popupContext->QueueDestroy();
			}

			popup->OnPopupClosed();

			return true;
		}
		
		int index = localPopupStack.IndexOf(popup);
		if (index < 0)
			return false;

		popup->isShown = false;
		popup->context = nullptr;
		localPopupStack.RemoveAt(index);

		FRootContext* rootContext = Object::CastTo<FRootContext>(GetRootContext());

		if (rootContext && rootContext->curFocusWidget != nullptr &&
			(rootContext->curFocusWidget == popup || rootContext->curFocusWidget->ParentExistsRecursive(popup.Get())))
		{
			if (localPopupStack.NotEmpty())
			{
				rootContext->SetFocusWidget(localPopupStack.Top().Get());
			}
			else if (popup->IsOfType<FMenuPopup>() && ((FMenuPopup*)popup.Get())->ownerItem != nullptr)
			{
				rootContext->SetFocusWidget(((FMenuPopup*)popup.Get())->ownerItem.Get());
			}
			else if (owningWidget)
			{
				rootContext->SetFocusWidget(owningWidget.Get());
			}
		}

		MarkLayoutDirty();

		popup->OnPopupClosed();

		return true;
	}

	void FFusionContext::SetDefaultStyleSet(FStyleSet* styleSet)
	{
		defaultStyleSet = styleSet;
		MarkLayoutDirty();
	}

	void FFusionContext::SetClearScreen(bool set)
	{
		if (clearScreen != set)
		{
			clearScreen = set;
		}
	}

	void FFusionContext::SetClearColor(const Color& color)
	{
		clearColor = color;
	}

	FPainter* FFusionContext::GetPainter()
	{
		return nullptr;
	}

	void FFusionContext::SetFocusWidget(FWidget* focusWidget)
	{
		widgetToFocus = focusWidget;

		if (IsNativeContext() && !IsRootContext())
		{
			FusionApplication::Get()->GetRootContext()->SetFocusWidget(focusWidget);
		}

		SetContextFocus();
	}

	bool FFusionContext::IsPopupWindow() const
	{
		return owningWidget != nullptr && owningWidget->IsOfType<FPopup>();
	}

	Vec2 FFusionContext::GlobalToScreenSpacePosition(Vec2 pos)
	{
		return pos;
	}

	Vec2 FFusionContext::ScreenToGlobalSpacePosition(Vec2 pos)
	{
		return pos;
	}

	FWidget* FFusionContext::HitTest(Vec2 mousePosition, bool requireFocus)
	{
		FWidget* hoveredWidget;

		Vec2 screenPos = GlobalToScreenSpacePosition(mousePosition);

		Array<Ref<FFusionContext>> contexts = childContexts;

		if (!requireFocus)
		{
			contexts.Sort([&](Ref<FFusionContext> lhs, Ref<FFusionContext> rhs)
				{
					return lhs->GetZOrder() > rhs->GetZOrder();
				});

			int minZOrder = NumericLimits<int>::Max();

			for (Ref<FFusionContext> context : contexts)
			{
				if (context->IsGhosted())
					continue;
				minZOrder = Math::Min(minZOrder, context->GetZOrder());
			}

			if (minZOrder == NumericLimits<int>::Max() || GetZOrder() < minZOrder)
			{
				Rect windowRect = Rect::FromSize(Vec2(), GetAvailableSize());
				if (windowRect.Contains(mousePosition))
				{
					for (int i = localPopupStack.GetSize() - 1; i >= 0; --i)
					{
						hoveredWidget = localPopupStack[i]->HitTest(mousePosition);
						if (hoveredWidget || localPopupStack[i]->m_BlockInteraction)
						{
							return hoveredWidget;
						}
					}

					if (owningWidget)
					{
						hoveredWidget = owningWidget->HitTest(mousePosition);
						if (hoveredWidget)
						{
							return hoveredWidget;
						}
					}
				}
			}
		}

		for (int i = contexts.GetSize() - 1; i >= 0; --i)
		{
			Ref<FFusionContext> context = contexts[i];
			if (context->IsGhosted())
			{
				continue;
			}

			hoveredWidget = context->HitTest(context->ScreenToGlobalSpacePosition(screenPos));

			if (context->IsPopupWindow())
			{
				FPopup* popup = static_cast<FPopup*>(context->owningWidget.Get());
				if (popup->BlockInteraction())
				{
					return hoveredWidget;
				}
			}

			if (requireFocus && !context->IsFocused())
			{
				continue;
			}

			if (hoveredWidget)
			{
				return hoveredWidget;
			}
		}

		{
			Rect windowRect = Rect::FromSize(Vec2(), GetAvailableSize());
			if (!windowRect.Contains(mousePosition))
				return nullptr;

			for (int i = localPopupStack.GetSize() - 1; i >= 0; --i)
			{
				hoveredWidget = localPopupStack[i]->HitTest(mousePosition);
				if (hoveredWidget || localPopupStack[i]->m_BlockInteraction)
				{
					return hoveredWidget;
				}
			}
		}

		if (!owningWidget)
			return nullptr;

		hoveredWidget = owningWidget->HitTest(mousePosition);
		return hoveredWidget;
	}

	void FFusionContext::EmplaceFrameAttachments()
	{
		ZoneScoped;

		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->EmplaceFrameAttachments();
		}
	}

	void FFusionContext::EnqueueScopes()
	{
		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->EnqueueScopes();
		}
	}

	void FFusionContext::SetDrawListMask(RHI::DrawListMask& outMask)
	{
		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->SetDrawListMask(outMask);
		}
	}

	void FFusionContext::EnqueueDrawPackets(RHI::DrawListContext& drawList, u32 imageIndex)
	{
		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->EnqueueDrawPackets(drawList, imageIndex);
		}
	}

	void FFusionContext::SetDrawPackets(RHI::DrawListContext& drawList)
	{
		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->SetDrawPackets(drawList);
		}
	}

	void FFusionContext::OnStyleSetDeregistered(FStyleSet* styleSet)
	{
		if (owningWidget != nullptr)
		{
			owningWidget->ClearStyle();
		}

		for (Ref<FFusionContext> childContext : childContexts)
		{
			childContext->OnStyleSetDeregistered(styleSet);
		}
	}

	void FFusionContext::NotifyWindowEvent(FEventType eventType, FNativeContext* nativeContext)
	{
		ZoneScoped;

		if (!owningWidget)
			return;

		FNativeEvent exposedEvent{};
		exposedEvent.type = eventType;
		exposedEvent.direction = FEventDirection::TopToBottom;
		exposedEvent.nativeContext = nativeContext;
		exposedEvent.sender = owningWidget.Get();

		owningWidget->HandleEvent(&exposedEvent);

		for (int i = localPopupStack.GetSize() - 1; i >= 0; --i)
		{
			if (i > localPopupStack.GetSize())
				continue;

			exposedEvent.Reset();
			exposedEvent.sender = owningWidget.Get();

			localPopupStack[i]->HandleEvent(&exposedEvent);
		}

		for (int i = childContexts.GetSize() - 1; i >= 0; --i)
		{
			exposedEvent.Reset();

			exposedEvent.sender = childContexts[i]->owningWidget.Get();
			if (exposedEvent.sender == nullptr)
				continue;

			exposedEvent.sender->HandleEvent(&exposedEvent);
		}
	}

} // namespace CE
