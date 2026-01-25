#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API FusionSDFFontAtlasWindow : public FToolWindow
    {
        CE_CLASS(FusionSDFFontAtlasWindow, FToolWindow)
    protected:

        FusionSDFFontAtlasWindow();

        void Construct() override;

    public: // - Public API -

        static Ref<FusionSDFFontAtlasWindow> Show();

    private: // - Internal -

        void OnPostComputeLayout() override;

        void OnPaint(FPainter* painter) override;

        FComboBox* fontComboBox = nullptr;
        FComboBox* fontPageComboBox = nullptr;
        FStyledWidget* background = nullptr;
        FStyledWidget* atlasViewport = nullptr;

        Vec2i imageAtlasSize = Vec2i();

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FusionSDFFontAtlasWindow.rtti.h"
