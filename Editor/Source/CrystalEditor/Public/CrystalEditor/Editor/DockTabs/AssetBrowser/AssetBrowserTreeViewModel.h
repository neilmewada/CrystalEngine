#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowserTreeViewModel : public FTreeViewModel
    {
        CE_CLASS(AssetBrowserTreeViewModel, FTreeViewModel)
    protected:

        AssetBrowserTreeViewModel();

        void OnBeforeDestroy() override;
        
    public:

        virtual ~AssetBrowserTreeViewModel();

        void Init();

        FModelIndex GetParent(const FModelIndex& index) override;

        FModelIndex GetIndex(u32 row, u32 column, const FModelIndex& parent) override;

        FModelIndex FindIndex(PathTreeNode* node);

        u32 GetRowCount(const FModelIndex& parent) override;

        u32 GetColumnCount(const FModelIndex& parent) override;

        void SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent) override;

    private:


    };
    
} // namespace CE

#include "AssetBrowserTreeViewModel.rtti.h"
