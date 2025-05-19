#pragma once

namespace CE
{

    CLASS()
    class FUSION_API FListWidget : public FStyledWidget
    {
        CE_CLASS(FListWidget, FStyledWidget)
    public:

        typedef Delegate<FListItemWidget&(FListItem*, FListWidget*)> GenerateRowCallback;

        bool HasScrollBox() const { return scrollBox != nullptr; }

        FScrollBox& GetScrollBox() const { return *scrollBox; }

        void SelectItem(int index);

        FListItemWidget* GetSelectedItem();

        int GetSelectedItemIndex();

    protected:

        FListWidget();

        void Construct() override final;

        void OnPostComputeLayout() override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        void OnItemSelected(FListItemWidget* selectedItem);

        void RegenerateRows();

        FIELD()
        GenerateRowCallback m_GenerateRowDelegate;

        FListWidgetContainer* content = nullptr;
        FScrollBox* scrollBox = nullptr;

        Array<FListItemWidget*> itemWidgets;
        Array<FListItemWidget*> selectedItems;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(FSelectionMode, SelectionMode);

        FUSION_PROPERTY_WRAPPER(Gap, content);

        FUSION_DATA_PROPERTY(Array<FListItem*>, ItemList);

        FUSION_EVENT(ScriptEvent<void(FListWidget*)>, OnSelectionChanged);

        Self& GenerateRowDelegate(const GenerateRowCallback& callback);

        FUSION_WIDGET;
        friend class FListItemWidget;
        friend class FListWidgetStyle;
    };
    
}

#include "FListWidget.rtti.h"
