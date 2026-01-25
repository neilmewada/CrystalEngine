#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FTreeViewCell : public FCompoundWidget
    {
        CE_CLASS(FTreeViewCell, FCompoundWidget)
    public:

        // - Public API -

        FImage* GetIcon() const { return icon; }

        void StartEditing();

        Ref<FTreeViewRow> GetRow() { return row.Lock(); }

    protected:

        FTreeViewCell();

        void Construct() override;

        FImageButton* arrowIcon = nullptr;
        FImage* icon = nullptr;
        FLabel* label = nullptr;
        FHorizontalStack* contentStack = nullptr;

        Ref<FTextInput> textInput;
        WeakRef<FTreeViewRow> row;

    public: // - Fusion Properties -

        FUSION_PROPERTY_WRAPPER(Text, label);
        FUSION_PROPERTY_WRAPPER(FontSize, label);
        FUSION_PROPERTY_WRAPPER(Foreground, label);

        FUSION_PROPERTY_WRAPPER2(Enabled, arrowIcon, ArrowEnabled);
        FUSION_PROPERTY_WRAPPER2(Visible, arrowIcon, ArrowVisible);

        FUSION_PROPERTY_WRAPPER2(Padding, contentStack, ContentPadding);

        FUSION_PROPERTY_WRAPPER2(Enabled, icon, IconEnabled);
        FUSION_PROPERTY_WRAPPER2(Background, icon, IconBackground);

        Self& IconWidth(f32 value);
        Self& IconHeight(f32 value);

        FUSION_EVENT(ScriptDelegate<void()>, OnToggleExpansion);
        FUSION_EVENT(ScriptDelegate<void(FTreeViewCell&, const String&)>, OnLabelEdited);

        bool ArrowExpanded();
        Self& ArrowExpanded(bool expanded);

        FUSION_WIDGET;
        friend class FTreeViewContainer;
        friend class FTreeViewRow;
    };
    
}

#include "FTreeViewCell.rtti.h"
