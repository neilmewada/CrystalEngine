#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ActorComponentTreeViewModel : public FTreeViewModel
    {
        CE_CLASS(ActorComponentTreeViewModel, FTreeViewModel)
    protected:

        ActorComponentTreeViewModel();

	public:

        FModelIndex GetParent(const FModelIndex& index) override;
        FModelIndex GetIndex(u32 row, u32 column, const FModelIndex& parent) override;
        u32 GetRowCount(const FModelIndex& parent) override;
        u32 GetColumnCount(const FModelIndex& parent) override;
        void SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent) override;

    private:

        
    };
    
} // namespace CE

#include "ActorComponentTreeViewModel.rtti.h"
