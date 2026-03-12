#include "FusionCore.h"

namespace CE
{

    FSurface::FSurface()
    {
        layerTree = CreateDefaultSubobject<FLayerTree>("LayerTree");
    }

    void FSurface::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

	    for (int i = 0; i < viewConstantBuffers.GetSize(); ++i)
	    {
            delete viewConstantBuffers[i]; viewConstantBuffers[i] = nullptr;
	    }

        delete viewSrg; viewSrg = nullptr;
    }

    void FSurface::GetDrawListMask(RHI::DrawListMask& drawListMask)
    {
		if (drawListTag.IsValid())
        {
            drawListMask.Set(drawListTag);
        }
    }

    void FSurface::AddChildSurface(Ref<FSurface> childSurface)
    {
		if (childrenSurfaces.Exists(childSurface))
        {
            return;
        }

		childrenSurfaces.Add(childSurface);
		childSurface->parentSurface = this;
    }

    bool FSurface::IsNativeSurface()
    {
		return IsOfType<FNativeSurface>();
    }

    void FSurface::SetOwningWidget(Ref<FWidget> widget)
    {
		if (rootWidget == widget)
            return;

		if (rootWidget)
        {
            rootWidget->SetParentSurfaceRecursive(nullptr);
        }

		rootWidget = widget;

        if (rootWidget)
		{
			rootWidget->SetParentSurfaceRecursive(this);

            AddPendingLayoutRoot(rootWidget);

            rootWidget->UpdateBoundaryFlags();

            rootWidget->MarkLayoutDirty();
            rootWidget->MarkPaintDirty();
		}
    }

    Ref<FStyleSet> FSurface::GetStyleSet()
    {
        if (styleSet.IsValid())
			return styleSet;
        if (Ref<FSurface> parent = parentSurface.Lock())
        {
            return parent->GetStyleSet();
        }
        return FApplication::Get()->GetDefaultStyleSet();
    }

    void FSurface::MarkLayerTreeDirty()
    {
        layerTree->MarkSyncNeeded();
    }

    void FSurface::AddPendingLayoutRoot(Ref<FWidget> layoutRoot)
    {
        if (!layoutRoot)
            return;

		if (pendingLayoutRootIds.Exists(layoutRoot->GetUuid()))
            return;

		pendingLayoutRoots.Add(layoutRoot);
        pendingLayoutRootIds.Add(layoutRoot->GetUuid());
    }

    void FSurface::Initialize()
    {
        for (u32 i = 0; i < viewConstantBuffers.GetSize(); i++)
        {
            RHI::BufferDescriptor desc{};
            desc.name = "FSurface ViewConstants";
            desc.bindFlags = BufferBindFlags::ConstantBuffer;
            desc.bufferSize = sizeof(RPI::PerViewConstants);
            desc.structureByteStride = desc.bufferSize;
            desc.defaultHeapType = MemoryHeapType::Upload;

            viewConstantBuffers[i] = RHI::gDynamicRHI->CreateBuffer(desc);
        }

        RHI::ShaderResourceGroupLayout viewSrgLayout;
        viewSrgLayout.srgType = SRGType::PerView;
        viewSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_PerViewData",
                0,
                RHI::ShaderResourceType::ConstantBuffer,
                RHI::ShaderStage::Vertex | RHI::ShaderStage::Fragment
            )
        );

        RHI::ShaderResourceGroupDescriptor srgDesc{};
        srgDesc.name = "FSurface SRG_PerView";
        srgDesc.layout = viewSrgLayout;

        viewSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(srgDesc);

        for (u32 i = 0; i < viewConstantBuffers.GetSize(); i++)
        {
            viewSrg->Bind(i, "_PerViewData", viewConstantBuffers[i]);
        }

        viewSrg->FlushBindings();
    }

    void FSurface::TickSurface(f32 deltaTime)
    {
		if (!rootWidget)
            return;

        // - Layout

        try
        {
            HashSet<FWidget*> pendingSet;
            for (auto& root : pendingLayoutRoots)
                pendingSet.Add(root.Get());

            // Remove any root whose ancestor is also pending
            pendingLayoutRoots.RemoveAll([&](Ref<FWidget> root)
                {
                    Ref<FWidget> ancestor = root->GetParentWidget();
                    while (ancestor != nullptr)
                    {
                        if (pendingSet.Exists(ancestor.Get()))
                        {
							pendingLayoutRootIds.Remove(root->GetUuid());
	                        return true;
                        }
                        ancestor = ancestor->GetParentWidget();
                    }
                    return false;
                });

			for (int i = pendingLayoutRoots.GetSize() - 1; i >= 0; i--)
            {
				Ref<FWidget> root = pendingLayoutRoots[i];
				pendingLayoutRoots.RemoveAt(i);
				if (!root)
                    continue;

                pendingLayoutRootIds.Remove(root->GetUuid());

				if (root->IsFaulted())
                    continue;

				Vec2 availableSize = GetAvailableSize();
                if (Ref<FWidget> parentWidget = root->GetParentWidget())
                {
					availableSize.x = Math::Max(0.0f, parentWidget->GetLayoutSize().x - parentWidget->Padding().left - parentWidget->Padding().right);
					availableSize.y = Math::Max(0.0f, parentWidget->GetLayoutSize().y - parentWidget->Padding().top - parentWidget->Padding().bottom);
                }
                
				root->MeasureContent(availableSize);
				root->ArrangeContent(availableSize);
            }
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while calculating Layout.\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        // - Layer Tree Sync

        layerTree->DoSyncIfNeeded(rootWidget.Get());

        // - Paint

        try
        {
            layerTree->DoPaintIfNeeded();
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception in FSurface::TickSurface on class {}, while painting. Stack Trace:\n{}", GetClass()->GetName().GetLastComponent(), exception.GetStackTraceString(true));
        }

        // - Composite


    }

    void FSurface::UpdateViewConstantBuffer(u32 imageIndex)
    {
        ZoneScoped;

        viewConstantBuffers[imageIndex]->UploadData(&viewConstants, sizeof(viewConstants));

        for (int i = 0; i < childrenSurfaces.GetSize(); i++)
        {
            childrenSurfaces[i]->UpdateViewConstantBuffer(imageIndex);
        }
    }

    void FSurface::OnSurfaceResize()
    {
        
    }

} // namespace CE

