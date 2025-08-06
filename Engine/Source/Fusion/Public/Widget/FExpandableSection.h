#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FExpandableSection : public FCompoundWidget
    {
        CE_CLASS(FExpandableSection, FCompoundWidget)
    protected:

        FExpandableSection();

        void Construct() override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

    public: // - Public API -

    protected: // - Internal -

        FButton* header = nullptr;
        FStyledWidget* content = nullptr;
        FImage* arrowIcon = nullptr;
        FLabel* titleLabel = nullptr;

    public: // - Fusion Properties - 

        Self& ExpandableContent(FWidget& widget);

        FUSION_LAYOUT_PROPERTY(bool, Expanded);

        FUSION_PROPERTY_WRAPPER2(Text, titleLabel, Title);
        FUSION_PROPERTY_WRAPPER2(FontSize, titleLabel, TitleFontSize);
        FUSION_PROPERTY_WRAPPER2(FillRatio, content, ContentFillRatio);

        FUSION_EVENT(ScriptEvent<void(FExpandableSection*)>, OnExpansionChanged);

        FUSION_WIDGET;
        friend class FExpandableSectionStyle;
    };
    
}

#include "FExpandableSection.rtti.h"
