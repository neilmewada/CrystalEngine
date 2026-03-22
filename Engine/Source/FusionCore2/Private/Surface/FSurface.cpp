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

        for (int i = 0; i < quadBuffersPerImage.GetSize(); i++)
        {
            delete quadBuffersPerImage[i]; quadBuffersPerImage[i] = nullptr;
        }

        for (u32 i = 0; i < renderSnapshots.GetSize(); i++)
        {
            renderSnapshots[i] = nullptr;
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

    FWidget* FSurface::HitTestWidget(Vec2 pos, FWidget* widget)
    {
        if (widget == nullptr)
        {
            widget = rootWidget.Get();
            if (!widget) return nullptr;

            // Convert surface space -> root layer space
            if (Ref<FLayer> rootLayer = layerTree->GetRootLayer())
                pos = rootLayer->GetGlobalTransform().Inverse().TransformPoint(pos);
        }

        if (!widget->IsEnabled() || widget->IsFaulted() || !widget->IsVisible())
            return nullptr;

        // Broad-phase: pos is in widget's own layer space, cachedLayerSpaceAABB is too
        if (!widget->cachedLayerSpaceAABB.Contains(pos))
            return nullptr;

        // Walk children last-to-first (last = painted on top = check first)
        for (int i = (int)widget->GetChildCount() - 1; i >= 0; i--)
        {
            FWidget* child = widget->GetChildAt(i).Get();
            if (!child) continue;

            Vec2 childPos;
            if (child->IsPaintBoundary())
            {
                // Child layer space = parent widget's local space numerically.
                // Convert: parent layer pos → child layer pos via parent widget's transform inverse.
                childPos = widget->cachedLayerSpaceTransform.Inverse().TransformPoint(pos);
            }
            else
            {
                childPos = pos;
            }

            if (FWidget* hit = HitTestWidget(childPos, child))
                return hit;
        }

        // Exact self hit test — convert layer pos → widget local space
        Vec2 localPos = widget->cachedLayerSpaceTransform.Inverse().TransformPoint(pos);
        if (widget->HitTest(localPos))
            return widget;

        return nullptr;
    }

    void FSurface::Initialize()
    {
        for (u32 i = 0; i < viewConstantBuffers.GetSize(); i++)
        {
            RHI::BufferDescriptor desc{};
            desc.name = "FSurface ViewConstants";
            desc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
            desc.bufferSize = sizeof(RPI::PerViewConstants);
            desc.structureByteStride = desc.bufferSize;
            desc.defaultHeapType = RHI::MemoryHeapType::Upload;

            viewConstantBuffers[i] = RHI::gDynamicRHI->CreateBuffer(desc);
        }

        RHI::ShaderResourceGroupLayout viewSrgLayout;
        viewSrgLayout.srgType = RHI::SRGType::PerView;
        viewSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_PerViewData",
                0,
                RHI::ShaderResourceType::ConstantBuffer,
                RHI::ShaderStage::Vertex | RHI::ShaderStage::Fragment | RHI::ShaderStage::Compute
            )
        );

        layerMatricesBuffers.Init("Layer Matrices", 4, RHI::BufferBindFlags::ConstantBuffer);
        drawItemBuffers.Init("Draw Items", 64, RHI::BufferBindFlags::StructuredBuffer);

        auto layerSrgLayout = FApplication::Get()->GetService<FRenderService>()->GetSubPassSrgLayout();

        UpdateLayerSrgs();

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

    void FSurface::Shutdown()
    {
        for (auto srg : layerSrgs)
        {
            delete srg;
        }
        layerSrgs.Clear();

        for (auto srg : drawItemSrgs)
        {
            delete srg;
        }
        drawItemSrgs.Clear();

        for (RHI::DrawPacket* drawPacket : drawPackets)
        {
            delete drawPacket;
        }
        drawPackets.Clear();
    }

    void FSurface::TickSurface(f32 deltaTime)
    {
		if (!rootWidget)
            return;

        ZoneScoped;

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

    void FSurface::RenderFrame(u32 frameIndex)
    {
        ZoneScoped;

        Ptr<FRenderSnapshot> renderSnapshot = renderSnapshots[frameIndex];
        
        constexpr RHI::IndexFormat kIndexFormat = sizeof(FUIIndex) == 4 ? RHI::IndexFormat::Uint32 : RHI::IndexFormat::Uint16;

        renderSnapshot->BuildSnapshot(layerTree);

        const u64 vertexArrayByteSize = renderSnapshot->vertexArray.GetCount() * sizeof(FUIVertex);
        const u64 indexArrayByteSize = renderSnapshot->indexArray.GetCount() * sizeof(FUIIndex);

        u64 totalQuadBufferSize = vertexArrayByteSize + indexArrayByteSize;
        u64 currentQuadBufferSize = 0;

        // - Geometry Buffer -
        
        quadBufferGrowRatio = Math::Clamp(quadBufferGrowRatio, 0.1f, 0.5f);

        if (quadBuffersPerImage[frameIndex] == nullptr || quadBuffersPerImage[frameIndex]->GetBufferSize() < totalQuadBufferSize)
        {
            RHI::Buffer* oldBuffer = quadBuffersPerImage[frameIndex];
            if (oldBuffer)
            {
	            currentQuadBufferSize = oldBuffer->GetBufferSize();
                RPISystem::Get().QueueDestroy(oldBuffer);
                quadBuffersPerImage[frameIndex] = nullptr;
            }

            RHI::BufferDescriptor desc{};
            desc.name = "QuadsBuffer";
            desc.bufferSize = desc.structureByteStride = Math::Max((u64)((f32)currentQuadBufferSize * quadBufferGrowRatio), totalQuadBufferSize);
            desc.defaultHeapType = MemoryHeapType::Upload;
            desc.bindFlags = RHI::BufferBindFlags::VertexBuffer | RHI::BufferBindFlags::IndexBuffer;

            quadBuffersPerImage[frameIndex] = gDynamicRHI->CreateBuffer(desc);
        }

        u8* data = nullptr;
        quadBuffersPerImage[frameIndex]->Map(0, quadBuffersPerImage[frameIndex]->GetBufferSize(), (void**)&data);
        {
            memcpy(data, renderSnapshot->vertexArray.GetData(), vertexArrayByteSize);
            memcpy(data + vertexArrayByteSize, renderSnapshot->indexArray.GetData(), indexArrayByteSize);
        }
        quadBuffersPerImage[frameIndex]->Unmap();

        vertexBufferViewPerImage[frameIndex] = RHI::VertexBufferView(quadBuffersPerImage[frameIndex], 0, vertexArrayByteSize, sizeof(FUIVertex));
        indexBufferViewPerImage[frameIndex] = RHI::IndexBufferView(quadBuffersPerImage[frameIndex], vertexArrayByteSize, indexArrayByteSize, kIndexFormat);

        // - Layer Matrices cbuffer -

        if (layerMatricesBuffers.GetElementCount() < renderSnapshot->matricesPerLayer.GetCount())
        {
            layerMatricesBuffers.GrowToFit(renderSnapshot->matricesPerLayer.GetCount());

            UpdateLayerSrgs();
        }
        
        layerMatricesBuffers.GetBuffer(frameIndex)->Map(0, renderSnapshot->matricesPerLayer.GetByteSize(), (void**)&data);
        {
            memcpy(data, renderSnapshot->matricesPerLayer.GetData(), renderSnapshot->matricesPerLayer.GetByteSize());
        }
        layerMatricesBuffers.GetBuffer(frameIndex)->Unmap();

        // - Draw Items buffer -

        const u64 drawItemCount = renderSnapshot->drawItemArray.GetCount();

        if (drawItemBuffers.GetElementCount() < drawItemCount)
        {
            drawItemBuffers.GrowToFit(drawItemCount);
        }

        if (drawItemCount > 0)
        {
            drawItemBuffers.GetBuffer(frameIndex)->Map(0, drawItemCount * sizeof(FUIDrawItem), (void**)&data);
            {
                memcpy(data, renderSnapshot->drawItemArray.GetData(), drawItemCount * sizeof(FUIDrawItem));
            }
            drawItemBuffers.GetBuffer(frameIndex)->Unmap();
        }

        UpdateDrawItemSrgs(frameIndex);

        // - View Constants -

        viewConstants.pixelResolution = drawableSize.ToVec2();
        viewConstants.projectionMatrix = Matrix4x4::OrthographicProjection(
            0, availableSize.x,
            0, availableSize.y,
            -1.0f, 1.0f
        );
        viewConstants.viewProjectionMatrix = viewConstants.projectionMatrix;
        viewConstants.viewMatrix = Matrix4x4::Identity();
        viewConstants.nearPlane = -1.0f;
        viewConstants.farPlane = 1.0f;

        viewConstantBuffers[frameIndex]->UploadData(&viewConstants, sizeof(viewConstants));
    }

    void FSurface::OnSurfaceResize()
    {
        
    }

    void FSurface::FlushDrawPackets(RHI::DrawListContext& drawList, u32 frameIndex)
    {
        ZoneScoped;

        Ptr<FRenderSnapshot> snapshot = renderSnapshots[frameIndex];

        drawPacketCount = 0;

        for (int i = 0; i < snapshot->renderPassArray.GetCount(); i++)
        {
            const auto& renderPass = snapshot->renderPassArray[i];

            SIZE_T layerIndex = renderPass.layerIndex;

            for (int j = 0; j < renderPass.drawCmdCount; j++)
            {
                int drawCmdIdx = renderPass.drawCmdStartIndex + j;
                FUIDrawCmd drawCmd = snapshot->drawCmdArray[drawCmdIdx];

                u32 globalVertexOffset = snapshot->vertexSplits[layerIndex].startIndex + drawCmd.vertexOffset;
                u32 globalIndexOffset = snapshot->indexSplits[layerIndex].startIndex + drawCmd.indexOffset;

                DrawPacket* packet = nullptr;

                if (drawPackets.GetSize() > drawPacketCount)
                {
                    Ref<FShader> shader = FApplication::Get()->GetService<FRenderService>()->GetMainShader();

                    packet = drawPackets[drawPacketCount];
                    RHI::DrawItem& drawItem = packet->drawItems[0];
                    *drawItem.indexBufferView = indexBufferViewPerImage[frameIndex];
                    drawItem.vertexBufferViews[0] = vertexBufferViewPerImage[frameIndex];

                    packet->shaderResourceGroups[0] = layerSrgs[layerIndex];
                    packet->shaderResourceGroups[1] = viewSrg;
                    packet->shaderResourceGroups[2] = drawItemSrgs[layerIndex];

                    drawItem.pipelineState = shader->GetDefaultPipeline();

                    drawItem.arguments = RHI::DrawIndexedArguments(1, 0, globalVertexOffset, drawCmd.indexCount, globalIndexOffset);

                    drawPacketCount++;
                }
                else
                {
                    Ref<FShader> shader = FApplication::Get()->GetService<FRenderService>()->GetMainShader();

                    RHI::DrawPacketBuilder builder{};
                    builder.AddDrawItem({
                        .stencilRef = 0,
                        .drawItemTag = drawListTag,
                        .indexBufferView = indexBufferViewPerImage[frameIndex],
                        .vertexBufferViews = { vertexBufferViewPerImage[frameIndex] },
                        .uniqueShaderResourceGroups = {},
                        .pipelineState = shader->GetDefaultPipeline(),
                        .drawFilterMask = RHI::DrawFilterMask::ALL
                        });

                    builder.AddShaderResourceGroup(layerSrgs[layerIndex]);
                    builder.AddShaderResourceGroup(viewSrg);
                    builder.AddShaderResourceGroup(drawItemSrgs[layerIndex]);

                    builder.SetDrawArguments(RHI::DrawIndexedArguments(1, 0, globalVertexOffset, drawCmd.indexCount, globalIndexOffset));

                    packet = builder.Build();
                    drawPackets.Add(packet);

                    drawPacketCount++;
                }

                drawList.AddDrawPacket(packet);
            }
        }

        for (Ref<FSurface> childSurface : childrenSurfaces)
        {
            childSurface->FlushDrawPackets(drawList, frameIndex);
        }
    }

    void FSurface::UpdateLayerSrgs()
    {
        ZoneScoped;

        auto layerSrgLayout = FApplication::Get()->GetService<FRenderService>()->GetSubPassSrgLayout();

        for (int i = 0; i < layerMatricesBuffers.GetElementCount(); i++)
        {
            RHI::ShaderResourceGroup* srg = nullptr;

            if (i < layerSrgs.GetSize())
            {
                srg = layerSrgs[i];
            }
            else
            {
                srg = gDynamicRHI->CreateShaderResourceGroup({ "LayerSrg", layerSrgLayout });
                layerSrgs.Add(srg);
            }

            for (int j = 0; j < RHI::Limits::MaxSwapChainImageCount; j++)
            {
                srg->Bind(j, "_LayerData", RHI::BufferView(layerMatricesBuffers.GetBuffer(j), i * sizeof(Matrix4x4), sizeof(Matrix4x4)));
            }

            srg->FlushBindings();
        }
    }

    void FSurface::UpdateDrawItemSrgs(u32 frameIndex)
    {
        ZoneScoped;

        auto objectSrgLayout = FApplication::Get()->GetService<FRenderService>()->GetObjectSrgLayout();

        Ptr<FRenderSnapshot> renderSnapshot = renderSnapshots[frameIndex];

        for (int i = 0; i < (int)renderSnapshot->drawItemSplits.GetCount(); i++)
        {
            RHI::ShaderResourceGroup* srg = nullptr;

            if (i < drawItemSrgs.GetSize())
            {
                srg = drawItemSrgs[i];
            }
            else
            {
                srg = gDynamicRHI->CreateShaderResourceGroup({ "DrawItemSrg", objectSrgLayout });
                drawItemSrgs.Add(srg);
            }

            const auto& split = renderSnapshot->drawItemSplits[i];
            const u64 byteOffset = split.startIndex * sizeof(FUIDrawItem);
            const u64 byteSize = Math::Max(split.count, (SIZE_T)1) * sizeof(FUIDrawItem);

            srg->Bind(frameIndex, "_DrawItems", RHI::BufferView(drawItemBuffers.GetBuffer(frameIndex), byteOffset, byteSize));
            srg->FlushBindings();
        }
    }
} // namespace CE

