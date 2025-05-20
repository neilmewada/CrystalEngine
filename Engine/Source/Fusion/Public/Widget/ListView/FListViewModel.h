#pragma once

namespace CE
{
    class FListViewRow;
    class FListView;

    CLASS(Abstract)
    class FUSION_API FListViewModel : public FDataModel
    {
        CE_CLASS(FListViewModel, FDataModel)
    protected:

        FListViewModel();
        
    public:

        virtual ~FListViewModel();

        virtual int GetRowCount() = 0;

        virtual void SetData(int row, FListViewRow& widget) = 0;

        virtual SubClass<FListViewRow> GetRowWidgetClass();

        virtual void OnListViewAssigned() {}

    protected:

        WeakRef<FListView> listView = nullptr;

        friend class FListView;
    };
    
} // namespace CE

#include "FListViewModel.rtti.h"
