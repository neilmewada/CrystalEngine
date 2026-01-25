#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FGameWindow : public FWindow
    {
        CE_CLASS(FGameWindow, FWindow)
    public:

        // - Public API -

        void SetScene(RPI::Scene* scene);

        RPI::Scene* GetScene() const { return scene; }

        bool IsEmbeddedViewport() const;

        RHI::DrawListContext& GetDrawListContext() { return drawList; }

        RHI::DrawListMask& GetDrawListMask() { return drawListMask; }
        void SetDrawListMask(const RHI::DrawListMask& mask) { this->drawListMask = mask; }

    protected:

        FGameWindow();

        void Construct() override;

        void OnBeginDestroy() override;

        virtual void OnSceneChanged() {}

        RPI::Scene* scene = nullptr;

        RHI::DrawListContext drawList{};
        RHI::DrawListMask drawListMask{};

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FGameWindow.rtti.h"
