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
	}

	bool FLayer::IsLayerDirty()
	{
		return isLayerDirty;
	}

	void FLayer::UpdateLayerSrg(u32 frameIndex)
	{
		Matrix4x4 transform = cachedGlobalTransform.ToMatrix4x4();

		buffersPerImage[frameIndex]->UploadData(&transform, sizeof(transform));

		for (int i = 0; i < children.GetSize(); i++)
		{
			children[i]->UpdateLayerSrg(frameIndex);
		}
	}

	void FLayer::SetLayerDirty(bool value)
	{
		isLayerDirty = false;
	}

	void FLayer::OnAfterConstruct()
	{
		Super::OnAfterConstruct();

		if (IsDefaultInstance())
			return;

		if (Ref<FRenderService> renderService = FApplication::Get()->GetService<FRenderService>())
		{
			layerSrg = gDynamicRHI->CreateShaderResourceGroup({ "Layer SRG", renderService->GetSubPassSrgLayout() });

			for (int i = 0; i < buffersPerImage.GetSize(); i++)
			{
				buffersPerImage[i] = gDynamicRHI->CreateBuffer({ "LayerBuffer", sizeof(Matrix4x4), sizeof(Matrix4x4),
					RHI::BufferBindFlags::ConstantBuffer, RHI::MemoryHeapType::Upload });

				layerSrg->Bind(i, "_LayerData", buffersPerImage[i]);
			}

			layerSrg->FlushBindings();
		}
	}

	void FLayer::OnBeforeDestroy()
	{
		Super::OnBeforeDestroy();

		for (int i = 0; i < buffersPerImage.GetSize(); i++)
		{
			delete buffersPerImage[i]; buffersPerImage[i] = nullptr;
		}

		delete layerSrg; layerSrg = nullptr;
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

		// Force re-paint of all children layers
		for (auto child : children)
		{
			child->DoPaint();
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
		widget->cachedGlobalTransform = this->cachedGlobalTransform * painter.GetCurrentTransform();

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
