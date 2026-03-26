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
		else
		{
			for (auto layer : children)
			{
				layer->DoPaintIfNeeded();
			}
		}
	}

	FAffineTransform FLayer::GetGlobalTransform()
	{
		FAffineTransform global = GetTransformInParentSpace();

		FLayer* parent = this->parent.Get();

		while (parent != nullptr)
		{
			global = parent->GetTransformInParentSpace() * global;

			parent = parent->parent.Get();
		}

		return global;
	}

	void FLayer::DoPaint()
	{
		if (Ref<FWidget> widget = GetOwningWidget())
		{
			FPainter painter{ this };

			drawList.Clear();
			splitPoints.Clear();

			// Default draw item always exists
			drawList.drawItemArray[0].clipRectIndex = -1;

			/*
			Vec2 halfSize = widget->GetLayoutSize() / 2.0f;
			Vec2 clipCenter = widget->GetLayoutPosition() + halfSize;

			drawList.clipRectArray.Insert(FUIClipRect{
				.clipInverseTransform = FAffineTransform::Translation(-clipCenter).ToMatrix4x4(),
				.cornerRadii = Vec4(),
				.clipHalfSize = halfSize,
			});
			*/

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

					layer->cachedTransformInParentLayerSpace = painter.GetCurrentTransform();

					layer->needsCompositing = true;
					layer->DoPaintIfNeeded();
				}
			}
			
			return;
		}

		needsCompositing = true;

		Vec2 pivot = widget->Pivot() * widget->GetLayoutSize();

		painter.PushTransform(
			FAffineTransform::Translation(widget->GetLayoutPosition()) *
			FAffineTransform::Translation(pivot) *
			widget->Transform() *
			FAffineTransform::Translation(-pivot)
		);

		widget->cachedLayerSpaceTransform = painter.GetCurrentTransform();

		// Own AABB: transform the [0,0 → layoutSize] rect into layer space
		widget->cachedLayerSpaceAABB = widget->cachedLayerSpaceTransform.TransformAABB(
			Rect(Vec2(0, 0), widget->GetLayoutSize())
		);

		widget->SetWidgetFlag(FWidgetFlags::PaintDirty, false);
		widget->Paint(painter);

		bool clipPushed = false;

		// TODO: Push Clip
		if (widget->ClipContent())
		{
			painter.PushClip(Rect(Vec2(), widget->GetLayoutSize()), widget->ClipShape());
			clipPushed = true;
		}

		for (u32 i = 0; i < widget->GetChildCount(); i++)
		{
			if (Ref<FWidget> child = widget->GetChildAt(i))
			{
				DoPaint(child.Get(), painter);

				// We only include children in AABB if clipping is disabled
				if (widget->ClipContent())
					continue;

				// painter.GetCurrentTransform() is always the parent widget's transform here,
				// whether child was paint boundary (no push/pop) or normal (pushed then popped).
				if (child->IsPaintBoundary())
				{
					// child->cachedLayerSpaceAABB is in child layer space; convert to this layer
					widget->cachedLayerSpaceAABB = Rect::Union(
						widget->cachedLayerSpaceAABB,
						painter.GetCurrentTransform().TransformAABB(child->cachedLayerSpaceAABB)
					);
				}
				else
				{
					widget->cachedLayerSpaceAABB = Rect::Union(
						widget->cachedLayerSpaceAABB,
						child->cachedLayerSpaceAABB
					);
				}
			}
		}

		// TODO: Pop Clip
		if (clipPushed)
		{
			painter.PopClip();
		}

		widget->PaintOverlay(painter);

		painter.PopTransform();
	}
} // namespace CE
