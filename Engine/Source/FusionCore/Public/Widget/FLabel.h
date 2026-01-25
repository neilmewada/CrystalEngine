#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FLabel : public FWidget
    {
        CE_CLASS(FLabel, FWidget)
    protected:

        FLabel();

        void CalculateIntrinsicSize() override;

        void OnPaint(FPainter* painter) override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

    public: // - Fusion Properties -

        FUSION_DATA_LAYOUT_PROPERTY(String, Text);

        FUSION_LAYOUT_PROPERTY(FFont, Font);
        FUSION_LAYOUT_PROPERTY(FWordWrap, WordWrap);

    	FUSION_PROPERTY(Color, Foreground);
        FUSION_PROPERTY(FPen, Underline);

        Self& FontFamily(const CE::Name& fontFamily);
        Self& FontSize(f32 fontSize);
        Self& Bold(bool bold);
        Self& Italic(bool italic);

        CE::Name FontFamily();
        f32 FontSize();
        bool Bold();
        bool Italic();

    protected:

        Array<Rect> underlineRects;
        bool textCacheDirty = true;

        FUSION_WIDGET;
    };
    
}

#include "FLabel.rtti.h"
