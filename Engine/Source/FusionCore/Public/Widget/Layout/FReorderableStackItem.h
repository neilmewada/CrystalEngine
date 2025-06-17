#pragma once

namespace CE
{
    class FReorderableStack;

    CLASS()
    class FUSIONCORE_API FReorderableStackItem : public FStyledWidget
    {
        CE_CLASS(FReorderableStackItem, FStyledWidget)
    protected:

        FReorderableStackItem();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

        void OnAttachedToParent(FWidget* parent) override;

        void OnPostComputeLayout() override;

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

        virtual bool CanBeDetached() { return false; }

        virtual Ref<FReorderableStackItem> DetachItem() { return nullptr; }

    protected: // - Internal -

        WeakRef<FReorderableStack> ownerStack;

        Vec2 startMousePos;
        Vec2 lastMousePos;
        f32 dragStartPosX = 0;
        bool dragging = false;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FReorderableStack;
    };
    
}

#include "FReorderableStackItem.rtti.h"
