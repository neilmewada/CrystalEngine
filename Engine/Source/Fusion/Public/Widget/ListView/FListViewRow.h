#pragma once

namespace CE
{
    class FListView;

    CLASS()
    class FUSION_API FListViewRow : public FStyledWidget
    {
        CE_CLASS(FListViewRow, FStyledWidget)
    protected:

        FListViewRow();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }
        bool SupportsKeyboardEvents() const override { return true; }

        bool IsHovered() const { return isHovered; }
        bool IsSelected() const { return isSelected; }
        bool IsAlternate() const { return isAlternate; }

        int GetRowIndex() const { return rowIndex; }

    protected: // - Internal -

        int rowIndex = -1;
        bool isAlternate = false;
        bool isHovered = false;
        bool isSelected = false;
        WeakRef<FListView> listView;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FListViewContainer;
        friend class FListView;
    };
    
}

#include "FListViewRow.rtti.h"
