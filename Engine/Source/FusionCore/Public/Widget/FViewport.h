#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FViewport : public FGameWindow
    {
        CE_CLASS(FViewport, FGameWindow)
    public:

        // - Public API -

        void RecreateFrameBuffer();

        Vec2i GetResolution() const { return currentSize; }

        u32 GetFrameCount() const { return frames.GetSize(); }

        RPI::Texture* GetFrame(u32 imageIndex) const { return frames[imageIndex]; }
        RHI::TextureView* GetFrameView(u32 imageIndex) const { return frameViews[imageIndex]; }

        void Release();

        RHI::ShaderResourceGroup* GetTextureSrg() const { return textureSrg; }

        u64 ComputeMemoryFootprint() override;

    protected:

        FViewport();

        ~FViewport();

		void OnBeginDestroy() override;

        void Construct() override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        void OnPostComputeLayout() override;

        void OnPaint(FPainter* painter) override;

        Vec2i currentSize = Vec2i(0, 0);

        StaticArray<RPI::Texture*, RHI::Limits::MaxSwapChainImageCount> frames{};
        StaticArray<RHI::TextureView*, RHI::Limits::MaxSwapChainImageCount> frameViews{};

        RHI::ShaderResourceGroup* textureSrg = nullptr;

        int imageIndex = -1;

    public: // - Fusion Properties -

        FUSION_PROPERTY(RHI::Format, ImageFormat);
        FUSION_PROPERTY(u8, SampleCount);
        FUSION_EVENT(ScriptEvent<>, OnFrameBufferRecreated);


        FUSION_WIDGET;
    };
    
}

#include "FViewport.rtti.h"
