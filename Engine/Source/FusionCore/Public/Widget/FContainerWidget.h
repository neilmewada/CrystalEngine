#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FContainerWidget : public FWidget
    {
        CE_CLASS(FContainerWidget, FWidget)
    public:

        FContainerWidget();

        u32 GetChildCount() const { return children.GetSize(); }

        Ref<FWidget> GetChild(u32 index) const { return children[index].Lock(); }

        void SetContextRecursively(FFusionContext* context) override;

        FWidget* HitTest(Vec2 localMousePos) override;

        bool ChildExistsRecursive(FWidget* child) override;

        void ApplyStyleRecursively() override;

        void HandleEvent(FEvent* event) override;

        void InsertChild(int index, FWidget* child);

        void MoveChildToIndex(FWidget* child, int index);

        void DestroyAllChildren();
        void QueueDestroyAllChildren();
        void RemoveAllChildren();

    protected:

        void OnPaint(FPainter* painter) override;

        void ClearStyle() override;

        bool TryAddChild(FWidget* child) override;

        bool TryRemoveChild(FWidget* child) override;

    	void OnChildWidgetDestroyed(FWidget* child) override;

    protected: // - Fields -

        FIELD()
        Array<WeakRef<FWidget>> children{};

    public: // - Fusion Properties -

        FUSION_PROPERTY(bool, ClipChildren);

        FUSION_PROPERTY(Color, DebugColor);

        FUSION_WIDGET;
    };

} // namespace CE

#include "FContainerWidget.rtti.h"