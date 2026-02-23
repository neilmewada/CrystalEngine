#include "FusionCore.h"

namespace CE
{

    FLayer::FLayer()
    {

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

    bool FLayer::MarkPaintDirty()
    {
		paintDirty = true;
    }
} // namespace CE

