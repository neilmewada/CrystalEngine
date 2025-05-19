#include "Fusion.h"

namespace CE
{

    FListViewModel::FListViewModel()
    {

    }

    FListViewModel::~FListViewModel()
    {
        
    }

    SubClass<FListViewRow> FListViewModel::GetRowWidgetClass()
    {
        return FListViewRow::StaticClass();
    }
} // namespace CE

