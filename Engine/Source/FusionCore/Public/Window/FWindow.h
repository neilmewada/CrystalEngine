#pragma once

namespace CE
{

    CLASS()
    class FUSIONCORE_API FWindow : public FStyledWidget
    {
        CE_CLASS(FWindow, FStyledWidget)
    public:

        FWindow();

        bool SupportsMouseEvents() const override { return true; }

        void ShowWindow();
        void HideWindow();

        PlatformWindow* GetPlatformWindow();

        virtual void SetWindowContent(FWidget& content);

        void SetWindowContent(Ref<FWidget> content)
        {
            SetWindowContent(*content);
        }

    protected:

        void OnPaint(FPainter* painter) override;

        virtual void OnMaximized() {}
        virtual void OnRestored() {}
        virtual void OnMinimized() {}

    public: // - Fusion Properties -

        FUSION_PROPERTY(f32, Opacity);

        FUSION_WIDGET;
    };
    
}

#include "FWindow.rtti.h"
