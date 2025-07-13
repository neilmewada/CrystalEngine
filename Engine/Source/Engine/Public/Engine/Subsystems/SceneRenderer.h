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

    private:

        Ref<CE::Scene> targetScene;
        StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount> outputImages;

        RHI::DrawListContext drawList{};
        RHI::DrawListMask drawListMask{};

    public:

    };
    
} // namespace CE

#include "SceneRenderer.rtti.h"
