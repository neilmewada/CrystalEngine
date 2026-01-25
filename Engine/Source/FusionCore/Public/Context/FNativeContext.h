#pragma once

namespace CE
{
	class RendererSubsystem;
	class FWindow;
	class FLayoutManager;
	class FWidget;
	class FusionRenderer;
	class FFusionContext;
	class FPainter;
	class FStyleSet;

	//! @brief Represents a native platform window.
	CLASS()
	class FUSIONCORE_API FNativeContext final : public FFusionContext, public ApplicationMessageHandler
	{
		CE_CLASS(FNativeContext, FFusionContext)
	public:

		FNativeContext();

		virtual ~FNativeContext();

		void OnAfterConstruct() override;

		static FNativeContext* Create(PlatformWindow* platformWindow, const String& name, FFusionContext* parentContext = nullptr);

		PlatformWindow* GetPlatformWindow() const { return platformWindow; }

		Name GetAttachmentId() const { return attachmentId; }

		f32 GetScaling() const override;

		int GetMultisamplingCount() const { return sampleCount; }
		void SetMultisamplingCount(int msaa);

		int GetZOrder() override;

		void SetContextFocus() override;

		bool IsFocused() const override;

		bool IsShown() const override;

		void TickInput() override;

		void DoLayout() override;

		void DoPaint() override;

		void PushNativePopup(FPopup* popup, Vec2 globalPosition, Vec2 size = Vec2()) override;

		void EmplaceFrameAttachments() override;

		void EnqueueScopes() override;

		void SetDrawListMask(RHI::DrawListMask& outMask) override;

		void EnqueueDrawPackets(RHI::DrawListContext& drawList, u32 imageIndex) override;

		void SetDrawPackets(RHI::DrawListContext& drawList) override;

		void OnWidgetDestroyed(FWidget* widget) override;

		FPainter* GetPainter() override;

		Vec2 GlobalToScreenSpacePosition(Vec2 pos) override;
		Vec2 ScreenToGlobalSpacePosition(Vec2 pos) override;

		void Maximize();
		void Minimize();
		void Restore();

		bool IsMaximized();
		bool IsMinimized();

		void SetWindowPosition(Vec2i newPos);

		//! @brief Returns the window size in Fusion units. This does not directly equal to the native window size because of DPI scaling.
		Vec2i GetWindowSize();

	protected:

		bool WindowDragHitTest(PlatformWindow* window, Vec2 position);

		void Init();

		void OnQueuedDestroy() override;

		void OnBeginDestroy() override;

		void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
		void OnWindowExposed(PlatformWindow* window) override;
		void OnWindowMoved(PlatformWindow* window, int x, int y) override;
		void OnWindowDestroyed(PlatformWindow* window) override;

		void OnWindowMaximized(PlatformWindow* window) override;
		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowMinimized(PlatformWindow* window) override;

		void UpdateViewConstants();

		PlatformWindow* platformWindow = nullptr;
		u32 windowDpi = 0;

		RHI::SwapChain* swapChain = nullptr;
		RHI::DrawListTag drawListTag = 0;
		int sampleCount = 1;
		Name attachmentId;

		Array<ImageScopeAttachmentDescriptor> shaderReadOnlyAttachmentDependencies;
		Array<ImageScopeAttachmentDescriptor> shaderWriteAttachmentDependencies;

		Vec2i windowPosToSet;
		bool updateWindowPos = false;
		
		FusionRenderer2* renderer2 = nullptr;
		FPainter* painter = nullptr;
		int windowId = -1;

		FUSION_FRIENDS;
		friend class CE::RendererSubsystem;
		friend class RenderingTests::RendererSystem;
	};

} // namespace CE

#include "FNativeContext.rtti.h"