#include "FusionCore.h"

namespace CE
{
	FLayer::FLayer()
	{
		
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

		if (widget != GetOwningWidget().Get() && widget->IsPaintBoundary())
		{
			if (Ref<FLayerTree> tree = ownerTree.Lock())
			{
				if (Ref<FLayer> layer = tree->FindLayerForWidget(widget->GetUuid()))
				{
					splitPoints.Add(drawList.GetCurrentDrawCmdCount());

					layer->cachedGlobalTransform = this->cachedGlobalTransform
						* painter.GetCurrentTransform()
						* FAffineTransform::Translation(widget->GetLayoutPosition())
						* widget->Transform();

					layer->DoPaint();
				}
			}

			return;
		}

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

		painter.PopTransform();
	}
} // namespace CE
