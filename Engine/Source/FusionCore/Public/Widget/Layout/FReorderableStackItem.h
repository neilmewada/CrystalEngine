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

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

    protected: // - Internal -

        WeakRef<FReorderableStack> ownerStack;

        f32 startMousePosX = 0;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FReorderableStack;
    };
    
}

#include "FReorderableStackItem.rtti.h"
