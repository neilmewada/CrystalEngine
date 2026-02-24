#include "FusionCore.h"

namespace CE
{

    FLayer::FLayer()
    {
        painter = CreateDefaultSubobject<FPainter>("Painter");
        painter->owningLayer = this;
    }

    void FLayer::AddChild(Ref<FLayer> childLayer)
    {
		if (!childLayer || children.Exists(childLayer))
        {
            return;
        }

        // Detach from previous parent first
        if (Ref<FLayer> oldParent = childLayer->parentLayer.Lock())
        {
            oldParent->RemoveChild(childLayer);
        }

		children.Add(childLayer);
		childLayer->parentLayer = this;
    }

    void FLayer::RemoveChild(Ref<FLayer> childLayer)
    {
        if (!childLayer || !children.Exists(childLayer))
        {
            return;
		}

		if (childLayer->parentLayer.Lock() == this)
        {
            childLayer->parentLayer = nullptr;
        }

		children.Remove(childLayer);
    }

    void FLayer::SetOwningWidget(Ref<FWidget> widget)
    {
		owningWidget = widget;
    }

    Ref<FSurface> FLayer::GetParentSurface()
    {
        if (Ref<FWidget> widget = owningWidget.Lock())
        {
            return widget->GetParentSurface();
        }
        return nullptr;
    }

    void FLayer::MarkPaintDirty()
    {
        needsRepaint = true;
    }
} // namespace CE

