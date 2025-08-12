#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API AddComponentTreeViewModel : public FTreeViewModel
    {
        CE_CLASS(AddComponentTreeViewModel, FTreeViewModel)
    protected:

        AddComponentTreeViewModel();

	public:

        FModelIndex GetParent(const FModelIndex& index) override;
        FModelIndex GetIndex(u32 row, u32 column, const FModelIndex& parent) override;
        u32 GetRowCount(const FModelIndex& parent) override;
        u32 GetColumnCount(const FModelIndex& parent) override;
        void SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent) override;

        bool IsFiltered() const { return filter.NotEmpty(); }

        void SetFilter(const String& filter);

    private:

        String filter = "";
        Array<ClassType*> filteredList;
    };
    
} // namespace CE

#include "AddComponentTreeViewModel.rtti.h"
