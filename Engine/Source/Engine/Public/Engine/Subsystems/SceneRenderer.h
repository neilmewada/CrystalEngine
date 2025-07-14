#pragma once

namespace CE
{
    class Scene;

    CLASS()
    class ENGINE_API SceneRenderer : public Object
    {
        CE_CLASS(SceneRenderer, Object)
    protected:

        SceneRenderer();
        
    public:

        virtual ~SceneRenderer();

        void SetScene(Ref<CE::Scene> scene);

        void SetOutputImage(int imageIndex, RHI::Texture* outputImage);

        Ref<CE::Scene> GetScene() { return targetScene; }

        RHI::Texture* GetOutputImage(int imageIndex) const { return outputImages[imageIndex]; }

        const auto& GetOutputImages() const { return outputImages; }

        RHI::DrawListContext& GetDrawListContext() { return drawList; }

        RHI::DrawListMask& GetDrawListMask() { return drawListMask; }

        void SetOneShot(bool set) { isOneShot = set; }

        bool IsOneShot() const { return isOneShot; }

    private:

        Ref<CE::Scene> targetScene;
        StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount> outputImages;
        bool isOneShot = false;
        int frameCounter = 0;

        RHI::DrawListContext drawList{};
        RHI::DrawListMask drawListMask{};

    public:

        ScriptEvent<void(Ref<CE::SceneRenderer>)> onRenderFinished;

        friend class RendererSubsystem;
        friend class SceneSubsystem;
    };
    
} // namespace CE

#include "SceneRenderer.rtti.h"
