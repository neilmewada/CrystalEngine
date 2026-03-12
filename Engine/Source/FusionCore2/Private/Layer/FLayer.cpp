#include "FusionCore.h"

namespace CE
{
	FLayer::FLayer()
	{
		
	}

	f32 FLayer::GetDpiScale()
	{
		if (Ref<FWidget> widget = GetOwningWidget())
		{
			if (Ref<FSurface> surface = widget->GetParentSurface())
			{
				return surface->GetDpiScale();
			}
		}
		return FApplication::Get()->GetDefaultDpiScale();
	}

	bool FLayer::NeedsRepaint()
	{
		if (Ref<FWidget> widget = GetOwningWidget())
		{
			return widget->IsPaintDirty();
		}
		return false;
	}

	void FLayer::DoPaintIfNeeded()
	{
		if (NeedsRepaint())
		{
			DoPaint();
		}

		for (auto child : children)
		{
			child->DoPaintIfNeeded();
		}
	}

	bool FLayer::IsLayerDirty()
	{
		return isLayerDirty;
	}

	void FLayer::SetLayerDirty(bool value)
	{
		isLayerDirty = false;
	}

	void FLayer::DoPaint()
	{
		if (Ref<FWidget> widget = GetOwningWidget())
		{
			FPainter painter{ this };

			drawList.Clear();
			splitPoints.Clear();

			DoPaint(widget.Get(), painter);

			drawList.Finalize();
		}
	}

	void FLayer::DoPaint(FWidget* widget, FPainter& painter)
	{
		if (widget == nullptr || widget->IsFaulted())
			return;

		// If a widget in the children hierarchy is a paint boundary
		if (widget != GetOwningWidget().Get() && widget->IsPaintBoundary())
		{
			if (Ref<FLayerTree> tree = ownerTree.Lock())
			{
				if (Ref<FLayer> layer = tree->FindLayerForWidget(widget->GetUuid()))
				{
					splitPoints.Add(drawList.GetCurrentDrawCmdCount());
					drawList.NewDrawCmd();

					layer->cachedGlobalTransform = this->cachedGlobalTransform
						* painter.GetCurrentTransform()
						* FAffineTransform::Translation(widget->GetLayoutPosition())
						* widget->Transform();

					layer->DoPaint();
				}
			}

			return;
		}

		isLayerDirty = true;

		painter.PushTransform(FAffineTransform::Translation(widget->GetLayoutPosition()) * widget->Transform());

		widget->SetWidgetFlag(FWidgetFlags::PaintDirty, false);
		widget->Paint(painter);

		for (u32 i = 0; i < widget->GetChildCount(); i++)
		{
			if (Ref<FWidget> child = widget->GetChildAt(i))
			{
				DoPaint(child.Get(), painter);
			}
		}

		widget->PaintOverlay(painter);

		painter.PopTransform();
	}
} // namespace CE
