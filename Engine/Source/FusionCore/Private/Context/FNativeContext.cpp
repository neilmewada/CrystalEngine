#include "FusionCore.h"

namespace CE
{
	static Array<int> availableWindowIds{};
	static int currentWindowId = 0;

	FNativeContext::FNativeContext()
	{
		isIsolatedContext = true;
	}

	FNativeContext::~FNativeContext()
	{
		if (platformWindow)
		{
			
		}
	}

	FNativeContext* FNativeContext::Create(PlatformWindow* platformWindow, const String& name, FFusionContext* parentContext)
	{
		ZoneScoped;

		Object* outer = parentContext;
		if (outer == nullptr)
		{
			outer = FusionApplication::TryGet();
		}
		if (outer == nullptr)
		{
			return nullptr;
		}

		FNativeContext* nativeContext = CreateObject<FNativeContext>(outer, name);
		nativeContext->platformWindow = platformWindow;
		nativeContext->windowDpi = platformWindow->GetWindowDpi();
		nativeContext->scaleFactor = FusionApplication::Get()->GetDefaultScalingFactor();

		if (parentContext)
		{
			parentContext->AddChildContext(nativeContext);
		}
		nativeContext->Init();
		return nativeContext;
	}

	void FNativeContext::Init()
	{
		ZoneScoped;

		if (availableWindowIds.NotEmpty())
		{
			windowId = availableWindowIds[0];
			availableWindowIds.RemoveAt(0);
		}
		else
		{
			windowId = currentWindowId++;
		}

		attachmentId = String::Format("Window_{}", windowId);

		drawListTag = RPI::RPISystem::Get().GetDrawListTagRegistry()->AcquireTag(attachmentId);

		platformWindow->SetHitTestDelegate(MemberDelegate(&Self::WindowDragHitTest, this));

		RHI::SwapChainDescriptor desc{};
		desc.imageCount = RHI::FrameScheduler::Get()->GetFramesInFlight();
		desc.preferredFormats = { RHI::Format::R8G8B8A8_UNORM, RHI::Format::B8G8R8A8_UNORM };
		desc.useMailboxMode = FusionApplication::Get()->IsMailboxPresentationPreferred();
		platformWindow->GetDrawableWindowSize(&desc.preferredWidth, &desc.preferredHeight);

		swapChain = RHI::gDynamicRHI->CreateSwapChain(platformWindow, desc);

		PlatformApplication::Get()->AddMessageHandler(this);

		//renderer = CreateObject<FusionRenderer>(this, "FusionRenderer");
		renderer2 = CreateObject<FusionRenderer2>(this, "FusionRenderer2");

		UpdateViewConstants();

		FusionRendererInitInfo rendererInfo;
		rendererInfo.fusionShader = FusionApplication::Get()->fusionShader2;
		rendererInfo.multisampling.sampleCount = 1;

		renderer2->SetDrawListTag(drawListTag);
		renderer2->Init(rendererInfo);

		painter = CreateObject<FPainter>(this, "FusionPainter");
		painter->renderer2 = renderer2;

		FusionApplication::Get()->nativeWindows.Add(this);
	}

	void FNativeContext::OnQueuedDestroy()
	{
		if (IsDefaultInstance())
			return;

		Super::OnQueuedDestroy();

		FusionApplication::Get()->nativeWindows.Remove(this);

		if (drawListTag.IsValid())
		{
			RPI::RPISystem::Get().GetDrawListTagRegistry()->ReleaseTag(drawListTag);
		}

		drawListTag = 0;

		PlatformApplication::Get()->RemoveMessageHandler(this);

		if (platformWindow)
		{
			platformWindow->Hide();
		}
	}

	void FNativeContext::OnBeginDestroy()
	{
		Super::OnBeginDestroy();

		PlatformApplication::Get()->RemoveMessageHandler(this);

		availableWindowIds.Add(windowId);
		windowId = -1;

		if (swapChain)
		{
			delete swapChain; swapChain = nullptr;
		}

		if (platformWindow)
		{
			PlatformApplication::Get()->DestroyWindow(platformWindow);
			platformWindow = nullptr;
		}
	}

	void FNativeContext::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
	{
		ZoneScoped;

		if (!platformWindow || !renderer2)
			return;
		if (platformWindow != window)
			return;

		availableSize = platformWindow->GetDrawableWindowSize().ToVec2() / GetScaling();

		if (swapChain)
		{
			//swapChain->Rebuild();
		}

		UpdateViewConstants();
		MarkLayoutDirty();

		NotifyWindowEvent(FEventType::NativeWindowResized, this);
	}

	void FNativeContext::OnWindowExposed(PlatformWindow* window)
	{
		ZoneScoped;

		if (!platformWindow || !renderer2)
			return;
		if (platformWindow != window)
			return;

		const Vec2 newSize = platformWindow->GetDrawableWindowSize().ToVec2();

		if (newSize / GetScaling() != availableSize)
		{
			availableSize = newSize / GetScaling();

			if (swapChain)
			{
				swapChain->Rebuild();
			}
		}
		
		UpdateViewConstants();
		MarkLayoutDirty();

		NotifyWindowEvent(FEventType::NativeWindowExposed, this);
	}

	void FNativeContext::OnWindowMoved(PlatformWindow* window, int x, int y)
	{
		ZoneScoped;

		if (!platformWindow || !renderer2)
			return;
		if (platformWindow != window)
			return;

		NotifyWindowEvent(FEventType::NativeWindowMoved, this);
	}

	void FNativeContext::OnWindowDestroyed(PlatformWindow* window)
	{
		if (platformWindow == window)
		{
			platformWindow = nullptr;
		}
	}

	void FNativeContext::OnWindowMaximized(PlatformWindow* window)
	{
		ZoneScoped;

		if (!platformWindow || !renderer2)
			return;
		if (platformWindow != window)
			return;

		if (owningWidget != nullptr && owningWidget->IsOfType<FWindow>())
		{
			FWindow* windowWidget = static_cast<FWindow*>(owningWidget.Get());
			windowWidget->OnMaximized();
		}
	}

	void FNativeContext::OnWindowRestored(PlatformWindow* window)
	{
		ZoneScoped;

		if (!platformWindow || !renderer2)
			return;
		if (platformWindow != window)
			return;

		if (owningWidget != nullptr && owningWidget->IsOfType<FWindow>())
		{
			FWindow* windowWidget = static_cast<FWindow*>(owningWidget.Get());
			windowWidget->OnRestored();
		}
	}

	void FNativeContext::OnWindowMinimized(PlatformWindow* window)
	{
		ZoneScoped;

		if (!platformWindow || !renderer2)
			return;
		if (platformWindow != window)
			return;

		if (owningWidget != nullptr && owningWidget->IsOfType<FWindow>())
		{
			FWindow* windowWidget = static_cast<FWindow*>(owningWidget.Get());
			windowWidget->OnMinimized();
		}
	}

	void FNativeContext::UpdateViewConstants()
	{
		ZoneScoped;

		u32 w = 0; u32 h = 0;
		platformWindow->GetDrawableWindowSize(&w, &h);

		f32 screenWidth = w / GetScaling();
		f32 screenHeight = h / GetScaling();
		
		viewConstants.viewMatrix = Matrix4x4::Identity();

		viewConstants.projectionMatrix =
			Matrix4x4::Scale(Vec3(1.0f / screenWidth * 2, 1.0f / screenHeight * 2, 1)) *
			Matrix4x4::Translation(Vec3(-screenWidth * 0.5f, -screenHeight * 0.5f, 0));

		viewConstants.viewProjectionMatrix = viewConstants.projectionMatrix * viewConstants.viewMatrix;
		viewConstants.viewPosition = Vec4(0, 0, 0, 0);
		viewConstants.pixelResolution = Vec2(screenWidth, screenHeight);

		if (renderer2)
		{
			renderer2->SetViewConstants(viewConstants);
		}
	}

	f32 FNativeContext::GetScaling() const
	{
        //return 1.0f;
		return (f32)windowDpi / 96.0f * scaleFactor;
	}

	void FNativeContext::SetMultisamplingCount(int msaa)
	{
		if (sampleCount == msaa)
			return;

		if (msaa == 1 || msaa == 2 || msaa == 4)
		{
			sampleCount = msaa;

			if (renderer2)
			{
				renderer2->multisampling.sampleCount = sampleCount;
			}

			FusionApplication::Get()->RequestFrameGraphUpdate();
		}
	}

	bool FNativeContext::IsFocused() const
	{
		return platformWindow->IsFocused();
	}

	bool FNativeContext::IsShown() const
	{
		return platformWindow->IsShown();
	}

	void FNativeContext::TickInput()
	{
		ZoneScoped;

		Super::TickInput();

		
	}

	void FNativeContext::DoLayout()
	{
		ZoneScoped;

		availableSize = platformWindow->GetDrawableWindowSize().ToVec2() / GetScaling();

		Super::DoLayout();
	}

	void FNativeContext::DoPaint()
	{
		ZoneScoped;

		Super::DoPaint();

		if (renderer2 && dirty)
		{
			renderer2->Begin();

			if (painter && owningWidget && owningWidget->Visible())
			{
				owningWidget->OnPaint(painter);
			}

			for (int i = 0; i < localPopupStack.GetSize(); ++i)
			{
				Ref<FPopup> popup = localPopupStack[i];

				if (popup->Visible())
				{
					if (popup->IsTranslationOnly())
					{
						renderer2->PushChildCoordinateSpace(popup->computedPosition + popup->Translation());
					}
					else
					{
						renderer2->PushChildCoordinateSpace(popup->GetLocalTransform());
					}

					popup->OnPaint(painter);

					renderer2->PopChildCoordinateSpace();
				}
			}

			renderer2->End();

			dirty = false;
		}
	}

	void FNativeContext::PushNativePopup(FPopup* popup, Vec2 globalPosition, Vec2 size)
	{
		ZoneScoped;
		// TODO: This is broken!

		if (!popup)
			return;

		for (Ref<FFusionContext> context : childContexts)
		{
			if (context->owningWidget == popup)
				return;
		}

		PlatformWindowInfo windowInfo;
		windowInfo.fullscreen = windowInfo.hidden = windowInfo.maximised = windowInfo.resizable = false;
		windowInfo.windowFlags = PlatformWindowFlags::PopupMenu | PlatformWindowFlags::DestroyOnClose | PlatformWindowFlags::SkipTaskbar;

		PlatformWindow* window = PlatformApplication::Get()->CreatePlatformWindow(popup->GetName().GetString(), size.x, size.y, windowInfo);
		Vec2i windowPos = this->platformWindow->GetWindowPosition() + globalPosition.ToVec2i();
		window->SetWindowPosition(windowPos);
		window->SetBorderless(true);
		if (!popup->AutoClose() || popup->BlockInteraction())
		{
			window->SetAlwaysOnTop(true);
		}
		window->SetHitTestDelegate(MemberDelegate(&Self::WindowDragHitTest, this));

		FNativeContext* popupContext = Create(window, popup->GetName().GetString(), this);
		// Always set the Context in widgets before calculating layout
		popupContext->SetOwningWidget(popup);

		if (size.x <= 0 || size.y <= 0)
		{
			popup->CalculateIntrinsicSize();

			size = popup->GetIntrinsicSize();
		}

		popup->computedPosition = Vec2();
		popup->computedSize = size;
		popup->PlaceSubWidgets();

		window->SetWindowSize(Vec2i((int)size.x, (int)size.y));

		popup->isShown = true;
		popup->isNativePopup = true;

		AddChildContext(popupContext);

		GetRootContext()->SetFocusWidget(popup);
		popupContext->MarkLayoutDirty();

		popup->ApplyStyle();
	}

	void FNativeContext::EmplaceFrameAttachments()
	{
		ZoneScoped;

		Super::EmplaceFrameAttachments();

		RHI::FrameScheduler* scheduler = RHI::FrameScheduler::Get();

		RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

		attachmentDatabase.EmplaceFrameAttachment(attachmentId, swapChain);
	}

	void FNativeContext::EnqueueScopes()
	{
		ZoneScoped;

		Super::EnqueueScopes();

		RHI::FrameScheduler* scheduler = RHI::FrameScheduler::Get();

		RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

		if (platformWindow != nullptr && !platformWindow->IsMinimized() && platformWindow->IsShown())
		{
			scheduler->BeginScope(attachmentId);
			{
				RHI::ImageScopeAttachmentDescriptor swapChainAttachment{};
				swapChainAttachment.attachmentId = attachmentId;
				if (clearScreen)
				{
					swapChainAttachment.loadStoreAction.clearValue = clearColor.ToVec4();
					swapChainAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
				}
				else
				{
					swapChainAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
				}
				
				swapChainAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
				swapChainAttachment.multisampleState.sampleCount = sampleCount;
				scheduler->UseAttachment(swapChainAttachment, RHI::ScopeAttachmentUsage::Color, RHI::ScopeAttachmentAccess::ReadWrite);

				for (const auto& shaderReadOnlyAttachmentDependency : shaderReadOnlyAttachmentDependencies)
				{
					scheduler->UseAttachment(shaderReadOnlyAttachmentDependency, RHI::ScopeAttachmentUsage::Shader, RHI::ScopeAttachmentAccess::Read);
				}

				for (const auto& shaderWriteAttachmentDependency : shaderWriteAttachmentDependencies)
				{
					scheduler->UseAttachment(shaderWriteAttachmentDependency, RHI::ScopeAttachmentUsage::Shader, RHI::ScopeAttachmentAccess::Write);
				}

				scheduler->PresentSwapChain(swapChain);
			}
			scheduler->EndScope();
		}
	}

	void FNativeContext::SetDrawListMask(RHI::DrawListMask& outMask)
	{
		Super::SetDrawListMask(outMask);

		if (drawListTag)
		{
			outMask.Set(drawListTag);
		}
	}

	void FNativeContext::EnqueueDrawPackets(RHI::DrawListContext& drawList, u32 imageIndex)
	{
		ZoneScoped;

		Super::EnqueueDrawPackets(drawList, imageIndex);


		{
			const auto& drawPackets = renderer2->FlushDrawPackets(imageIndex);

			for (RHI::DrawPacket* drawPacket : drawPackets)
			{
				drawList.AddDrawPacket(drawPacket);
			}
		}
	}

	void FNativeContext::SetDrawPackets(RHI::DrawListContext& drawList)
	{
		ZoneScoped;

		Super::SetDrawPackets(drawList);

		auto scheduler = RHI::FrameScheduler::Get();

		scheduler->SetScopeDrawList(attachmentId, &drawList.GetDrawListForTag(drawListTag));
	}

	void FNativeContext::OnWidgetDestroyed(FWidget* widget)
	{
		Super::OnWidgetDestroyed(widget);

		for (int i = 0; i < widgetsPressedPerMouseButton.GetSize(); i++)
		{
			if (widgetsPressedPerMouseButton[i] == widget)
			{
				widgetsPressedPerMouseButton[i] = nullptr;
			}
		}
	}

	FPainter* FNativeContext::GetPainter()
	{
		return painter;
	}

	Vec2 FNativeContext::GlobalToScreenSpacePosition(Vec2 pos)
	{
#if PLATFORM_MAC
		const f32 scaling = 96.0f / 72.0f / FusionApplication::Get()->GetDefaultScalingFactor(); // Mac input fix
		return platformWindow->GetWindowPosition().ToVec2() + pos / scaling;
#else
		return platformWindow->GetWindowPosition().ToVec2() + pos * GetScaling();
#endif
	}

	Vec2 FNativeContext::ScreenToGlobalSpacePosition(Vec2 pos)
	{
#if PLATFORM_MAC
		const f32 scaling = 96.0f / 72.0f / FusionApplication::Get()->GetDefaultScalingFactor(); // Mac input fix
		return (pos - platformWindow->GetWindowPosition().ToVec2()) * scaling;
#else
		return (pos - platformWindow->GetWindowPosition().ToVec2()) / GetScaling();
#endif
	}

	void FNativeContext::Maximize()
	{
		if (isDestroyed)
			return;

		platformWindow->Maximize();
	}

	void FNativeContext::Minimize()
	{
		if (isDestroyed)
			return;

		platformWindow->Minimize();
	}

	void FNativeContext::Restore()
	{
		if (isDestroyed)
			return;

		platformWindow->Restore();
	}

	bool FNativeContext::IsMaximized()
	{
		if (isDestroyed)
			return false;

		return platformWindow->IsMaximized();
	}

	bool FNativeContext::IsMinimized()
	{
		if (isDestroyed)
			return false;

		return platformWindow->IsMinimized();
	}

	bool FNativeContext::WindowDragHitTest(PlatformWindow* window, Vec2 position)
	{
		if (!window->IsBorderless() || IsPopupWindow())
			return false;

#if PLATFORM_MAC
		f32 macScaling = 96.0f / 72.0f / FusionApplication::Get()->GetDefaultScalingFactor(); // Mac input fix
		position *= macScaling;
#else
		position /= GetScaling();
#endif

		FWidget* hitWidget = HitTest(position);

		while (hitWidget != nullptr && !hitWidget->SupportsMouseEvents())
		{
			if (hitWidget->IsOfType<FTitleBar>() || hitWidget->WindowDragHitTest())
			{
				return true;
			}

			hitWidget = hitWidget->parent.Get();
		}

		return hitWidget != nullptr && hitWidget->IsOfType<FTitleBar>();
	}

} // namespace CE
