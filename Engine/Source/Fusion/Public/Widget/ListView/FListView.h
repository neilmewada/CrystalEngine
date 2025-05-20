#pragma once

namespace CE
{

    CLASS()
    class FUSION_API FListView : public FStyledWidget
    {
        CE_CLASS(FListView, FStyledWidget)
    protected:

        FListView();

        void Construct() override;

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

    public: // - Public API -

        int GetVisibleRowCount();

        FListViewRow* GetVisibleRow(int index);

        void SelectRow(FListViewRow* row, bool additional = false);

        const HashSet<int>& GetSelectedRowIndices() const { return selectedRows; }

        bool IsRowSelected(int index);

        void OnModelUpdate();

    protected: // - Internal -

        Ref<FScrollBox> scrollBox;
        Ref<FListViewContainer> container;

        HashSet<int> selectedRows;

    public: // - Fusion Properties - 

        FUSION_LAYOUT_PROPERTY(f32, RowHeight);
        FUSION_LAYOUT_PROPERTY(Delegate<f32(int)>, RowHeightDelegate);

        FUSION_PROPERTY(FSelectionMode, SelectionMode);

        FUSION_PROPERTY(Ref<FListViewModel>, Model);

        FUSION_LAYOUT_PROPERTY(Delegate<FListViewRow&()>, GenerateRowCallback);

        FUSION_EVENT(ScriptEvent<void(FListView*)>, OnSelectionChanged);

        FUSION_WIDGET;
        friend class FListViewContainer;
    };
    
}

#include "FListView.rtti.h"
