#include "FusionCore.h"

namespace CE
{

    FLayer::FLayer()
    {
        drawList = new FUIDrawList;
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

    void FLayer::DoPaintIfNeeded()
    {
        ZoneScoped;

        FPainter painter{ this };

        try
        {
            if (isPaintDirty)
            {
                painter.Begin();

                if (Ref<FWidget> widget = owningWidget.Get())
                {
                    widget->OnPaint(painter);
                }

                painter.End();

                isPaintDirty = false;
            }

            for (Ref<FLayer> child : children)
            {
                if (child->IsFaulted())
                    continue;

                child->DoPaintIfNeeded();
            }
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FLayer::DoPaint() while painting.\n{}", exception.GetStackTraceString(true));
            faulted = true;
        }
    }

    void FLayer::MarkPaintDirty()
    {
        isPaintDirty = true;
    }

} // namespace CE

