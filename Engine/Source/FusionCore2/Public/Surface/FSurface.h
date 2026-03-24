#pragma once

namespace CE
{
    class FWidget;

    CLASS(Abstract)
    class FUSIONCORE_API FSurface : public CE::Object
    {
        CE_CLASS(FSurface, CE::Object)
    protected:

        FSurface();
        
        void OnBeginDestroy() override;

    public: // - Public API -

        RHI::ScopeId GetScopeId() const { return scopeId; }

        RHI::DrawListTag GetDrawListTag() const { return drawListTag; }

        void GetDrawListMask(RHI::DrawListMask& drawListMask);

		void AddChildSurface(Ref<FSurface> childSurface);

        u32 GetChildSurfaceCount() const { return childrenSurfaces.GetSize(); }
        
        Ref<FSurface> GetChildSurface(u32 index) const { return childrenSurfaces[index]; }

        bool IsNativeSurface();

        void SetOwningWidget(Ref<FWidget> widget);

		Ref<FWidget> GetRootWidget() const { return rootWidget; }

		f32 GetDpiScale() const { return dpiScale; }

        Vec2 GetAvailableSize() const { return availableSize; }

        Ref<FStyleSet> GetStyleSet();

        void MarkLayerTreeDirty();

        Ref<FLayerTree> GetLayerTree() const { return layerTree; }

    public:

        // - Layout -

		void AddPendingLayoutRoot(Ref<FWidget> layoutRoot);

        void MarkRootLayoutDirty();

        // - Paint -

        // - Events -

        virtual void DispatchSurfaceUnfocusEvent();

        virtual void DispatchSurfaceFocusEvent();

        virtual void DispatchMouseEvents();
        virtual void DispatchKeyEvents();

        void ProcessReply(FWidget* sender, const FEventReply& reply);

        // - Input -

        virtual bool IsFocused() = 0;

        virtual Vec2 ScreenToSurfacePoint(Vec2 position) = 0;

        FWidget* HitTestWidget(Vec2 pos, FWidget* widget = nullptr);
    	
    	// - Lifecycle -

        virtual void Initialize();

        virtual void Shutdown();

        virtual void TickSurface(f32 deltaTime);

        virtual void RenderFrame(u32 frameIndex);

        virtual void OnSurfaceResize();

        // - Frame Graph -

        virtual void EmplaceFrameAttachments() = 0;

        virtual void EnqueueScopes() = 0;

        virtual void FlushDrawPackets(RHI::DrawListContext& drawList, u32 frameIndex);

    protected:

        // - Layer Srg -

        void UpdateLayerSrgs();

        void UpdateDrawItemSrgs(u32 frameIndex);

        RHI::DrawListTag drawListTag = 0;
		RHI::ScopeId scopeId;
        RHI::AttachmentID attachmentId;

        Array<RHI::DrawPacket*> drawPackets;
        u32 drawPacketCount = 0;

		HashSet<Uuid> pendingLayoutRootIds;
		Array<Ref<FWidget>> pendingLayoutRoots;

        Array<Ref<FSurface>> childrenSurfaces;
        
		WeakRef<FSurface> parentSurface;

        Ref<FLayerTree> layerTree;

        // - Event -

        KeyModifier keyModifierStates;
        
    	Array<WeakRef<FWidget>> hoveredWidgetStack;
        StaticArray<WeakRef<FWidget>, 5> pressedWidgetPerButton;
        WeakRef<FWidget> curFocusedWidget, nextFocusWidget;
        WeakRef<FWidget> capturedWidget;

        // - View Constants -
        RPI::PerViewConstants viewConstants{};
        StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> viewConstantBuffers;

        // - Render Data -
        StaticArray<Ptr<FRenderSnapshot>, RHI::Limits::MaxSwapChainImageCount> renderSnapshots;

        // - GPU Resources -
        StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> quadBuffersPerImage;
        StaticArray<RHI::VertexBufferView, RHI::Limits::MaxSwapChainImageCount> vertexBufferViewPerImage;
        StaticArray<RHI::IndexBufferView, RHI::Limits::MaxSwapChainImageCount> indexBufferViewPerImage;
        DynamicMultiBuffer<Matrix4x4> layerMatricesBuffers;
        DynamicMultiBuffer<FUIDrawItem> drawItemBuffers;

        // - SRGs -
        Array<RHI::ShaderResourceGroup*> layerSrgs{};
        Array<RHI::ShaderResourceGroup*> drawItemSrgs{};
        RHI::ShaderResourceGroup* viewSrg = nullptr;

        FIELD()
        f32 quadBufferGrowRatio = 0.2f;

        FIELD()
		f32 dpiScale = 1.0f;

        FIELD()
		Vec2 availableSize;

        FIELD()
        Vec2i drawableSize;

		FIELD()
		Ref<FWidget> rootWidget;

        FIELD()
        Ref<FStyleSet> styleSet;
    };
    
} // namespace CE

#include "FSurface.rtti.h"
