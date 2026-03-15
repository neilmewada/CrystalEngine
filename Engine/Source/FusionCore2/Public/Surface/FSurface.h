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

        void GetDrawListMask(RHI::DrawListMask& drawListMask);

		void AddChildSurface(Ref<FSurface> childSurface);

        bool IsNativeSurface();

        void SetOwningWidget(Ref<FWidget> widget);

		Ref<FWidget> GetRootWidget() const { return rootWidget; }

		f32 GetDpiScale() const { return dpiScale; }

        Vec2 GetAvailableSize() const { return availableSize; }

        Ref<FStyleSet> GetStyleSet();

        void MarkLayerTreeDirty();

    public:

        // - Layout -

		void AddPendingLayoutRoot(Ref<FWidget> layoutRoot);

        // - Paint -

    public: 
    	
    	// - Lifecycle -

        virtual void Initialize();

        virtual void Shutdown() = 0;

        virtual void TickSurface(f32 deltaTime);

        virtual void RenderFrame(u32 frameIndex);

        virtual void UpdateViewConstantBuffer(u32 frameIndex);

        virtual void OnSurfaceResize();

        // - Frame Graph -

        virtual void EmplaceFrameAttachments() = 0;

    protected:

        RHI::DrawListTag drawListTag = 0;
		RHI::ScopeId scopeId;

		HashSet<Uuid> pendingLayoutRootIds;
		Array<Ref<FWidget>> pendingLayoutRoots;

        Array<Ref<FSurface>> childrenSurfaces;
        
		WeakRef<FSurface> parentSurface;

        Ref<FLayerTree> layerTree;

        // - View Constants -
        RPI::PerViewConstants viewConstants{};
        StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> viewConstantBuffers;
        RHI::ShaderResourceGroup* viewSrg = nullptr;

        // - Render Data -
        StaticArray<Ptr<FRenderSnapshot>, RHI::Limits::MaxSwapChainImageCount> renderSnapshots;

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
