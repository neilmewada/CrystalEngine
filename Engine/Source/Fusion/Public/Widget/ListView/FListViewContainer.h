#pragma once

namespace CE
{
    class FListView;


    CLASS()
    class FUSION_API FListViewContainer : public FWidget
    {
        CE_CLASS(FListViewContainer, FWidget)
    protected:

        FListViewContainer();

        void Construct() override;

        void SetContextRecursively(FFusionContext* context) override;

        void OnPostComputeLayout() override;

        void OnPaint(FPainter* painter) override;

    public: // - Public API -

        FWidget* HitTest(Vec2 localMousePos) override;

        bool ChildExistsRecursive(FWidget* child) override;

        void CalculateIntrinsicSize() override;

        void PlaceSubWidgets() override;

        void ApplyStyleRecursively() override;

        void HandleEvent(FEvent* event) override;

    protected: // - Internal -

        void UpdateRows();

        void OnSelectionChanged();

        using ListViewRowList = StableDynamicArray<FListViewRow*, 64, false>;

        WeakRef<FListView> listView = nullptr;
        ListViewRowList children;

        f32 totalRowHeight = 0;

    public: // - Fusion Properties - 

        Self& ListView(Ref<FListView> listView);

        FUSION_WIDGET;
        friend class FListView;
    };
    
}

#include "FListViewContainer.rtti.h"
