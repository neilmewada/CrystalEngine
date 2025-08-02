#include <algorithm>

#include "FusionCore.h"

// Credit: Dear Imgui
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImRsqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define IM_FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > IM_FIXNORMAL2F_MAX_INVLEN2) inv_len2 = IM_FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0


namespace CE
{
    static inline float  ImRsqrt(float x) { return 1.0f / sqrtf(x); }

    static constexpr int CircleAutoSegmentMin = 4;
    static constexpr int CircleAutoSegmentMax = 512;
    static constexpr f32 MinFontSize = 4;

    static Vec2 RoundVector(Vec2 in)
    {
        return Vec2(Math::Round(in.x), Math::Round(in.y));
    }

    FusionRenderer2::FusionRenderer2()
    {
        // Can be overridden in Config
        circleSegmentMaxError = 0.2f;
    }

    FusionRenderer2::~FusionRenderer2()
    {
        
    }

    void FusionRenderer2::Init(const FusionRendererInitInfo& initInfo)
    {
        fusionShader = initInfo.fusionShader;
        multisampling = initInfo.multisampling;

        whitePixelUV = FusionApplication::Get()->GetImageAtlas()->GetWhitePixelUV();
        transparentPixelUV = FusionApplication::Get()->GetImageAtlas()->GetTransparentPixelUV();

        auto perObjectSrgLayout = fusionShader->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerObject);
        auto perViewSrgLayout = fusionShader->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerView);

        numFrames = RHI::FrameScheduler::Get()->GetFramesInFlight();

        perViewSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perViewSrgLayout);
        perObjectSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perObjectSrgLayout);

        objectDataBuffer.Init("ObjectData", initialObjectCount, numFrames);
        clipRectBuffer.Init("ClipRects", initialClipRectCount, numFrames);
        drawDataBuffer.Init("DrawData", initialDrawDataCount, numFrames);
        gradientBuffer.Init("GradientBuffer", initialGradientCount, numFrames);

        for (int i = 0; i < numFrames; ++i)
        {
            // - Create Buffers -

            RHI::BufferDescriptor viewConstantsBufferDesc{};
            viewConstantsBufferDesc.name = "ViewConstants";
            viewConstantsBufferDesc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
            viewConstantsBufferDesc.bufferSize = sizeof(RPI::PerViewConstants);
            viewConstantsBufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
            viewConstantsBufferDesc.structureByteStride = sizeof(RPI::PerViewConstants);

            viewConstantsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(viewConstantsBufferDesc);
            viewConstantsUpdateRequired[i] = true;

            RHI::BufferDescriptor quadMeshBufferDesc{};
            quadMeshBufferDesc.name = "Quad Meshes";
            quadMeshBufferDesc.bindFlags = RHI::BufferBindFlags::IndexBuffer | RHI::BufferBindFlags::VertexBuffer;
            quadMeshBufferDesc.bufferSize = initialMeshBufferSize;
            quadMeshBufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
            quadMeshBufferDesc.structureByteStride = quadMeshBufferDesc.bufferSize;

            quadsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(quadMeshBufferDesc);
            quadUpdatesRequired[i] = true;

            // - Bind Buffers -

            perViewSrg->Bind(i, "_PerViewData", viewConstantsBuffer[i]);

            perObjectSrg->Bind(i, "_Objects", objectDataBuffer.GetBuffer(i));
            perObjectSrg->Bind(i, "_ClipRects", clipRectBuffer.GetBuffer(i));
            perObjectSrg->Bind(i, "_DrawData", drawDataBuffer.GetBuffer(i));
            perObjectSrg->Bind(i, "_GradientKeys", gradientBuffer.GetBuffer(i));
        }

        perViewSrg->FlushBindings();
        perObjectSrg->FlushBindings();

        for (int i = 0; i < ArcFastTableSize; i++)
        {
            const float a = ((float)i * 2 * M_PI) / (float)ArcFastTableSize;
            arcFastVertex[i] = Vec2(cosf(a), sinf(a));
        }

        arcFastRadiusCutoff = ((circleSegmentMaxError) / (1 - cosf(Math::PI / Math::Max((float)(ArcFastTableSize), Math::PI))));
    }

    void FusionRenderer2::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        objectDataBuffer.Shutdown();
        clipRectBuffer.Shutdown();
        drawDataBuffer.Shutdown();
        gradientBuffer.Shutdown();

        while (freePackets.NotEmpty())
        {
            delete freePackets[0];
            freePackets.RemoveAt(0);
        }

    	for (int i = 0; i < numFrames; i++)
        {
            delete viewConstantsBuffer[i]; viewConstantsBuffer[i] = nullptr;
            delete quadsBuffer[i]; quadsBuffer[i] = nullptr;

            for (auto drawPacket : drawPacketsPerImage[i])
            {
                delete drawPacket;
            }
            drawPacketsPerImage[i].Clear();
        }

        delete perViewSrg; perViewSrg = nullptr;
        delete perObjectSrg; perObjectSrg = nullptr;
    }

    void FusionRenderer2::SetViewConstants(const RPI::PerViewConstants& viewConstants)
    {
        this->viewConstants = viewConstants;

        for (int i = 0; i < viewConstantsUpdateRequired.GetSize(); ++i)
        {
            viewConstantsUpdateRequired[i] = true;
        }
    }

    const Array<RHI::DrawPacket*>& FusionRenderer2::FlushDrawPackets(u32 imageIndex)
    {
        ZoneScoped;

        // - Destroy queued resources only when they are out of usage scope

        for (int i = (int)destructionQueue.GetSize() - 1; i >= 0; i--)
        {
            destructionQueue[i].frameCounter++;

            if (destructionQueue[i].frameCounter > (int)RHI::Limits::MaxSwapChainImageCount * 2)
            {
                delete destructionQueue[i].buffer;
                delete destructionQueue[i].srg;
                delete destructionQueue[i].material;
                destructionQueue.RemoveAt(i);
            }
        }

        if (viewConstantsUpdateRequired[imageIndex])
        {
            viewConstantsBuffer[imageIndex]->UploadData(&viewConstants, sizeof(viewConstants));

            viewConstantsUpdateRequired[imageIndex] = false;
        }

        if (quadUpdatesRequired[imageIndex])
        {
            // TODO: Implement support for more than 65,536 vertices!
            u32 totalSize = vertexArray.GetCount() * sizeof(FVertex) + indexArray.GetCount() * sizeof(FDrawIndex);

            if (totalSize > quadsBuffer[imageIndex]->GetBufferSize())
            {
                GrowQuadBuffer(totalSize);

                for (int i = 0; i < numFrames; ++i)
                {
                    for (RHI::DrawPacket* drawPacket : drawPacketsPerImage[i])
                    {
                        drawPacket->drawItems[0].debugName = debugName;

                        drawPacket->drawItems[0].vertexBufferViews[0] = RHI::VertexBufferView(quadsBuffer[i],
                                                                                              0,
                                                                                              vertexArray.GetCount() * sizeof(FVertex),
                                                                                              sizeof(FVertex));

                        drawPacket->drawItems[0].indexBufferView[0] = RHI::IndexBufferView(quadsBuffer[i],
                                                                                           vertexArray.GetCount() * sizeof(FVertex),
                                                                                           indexArray.GetCount() * sizeof(FDrawIndex),
                                                                                           sizeof(FDrawIndex) == 2 ? RHI::IndexFormat::Uint16 : RHI::IndexFormat::Uint32);
                    }
                }
            }

            if (totalSize > 0)
            {
                void* data;
                quadsBuffer[imageIndex]->Map(0, totalSize, &data);
                {
                    memcpy((u8*)data, 
                        vertexArray.GetData(), 
                        vertexArray.GetCount() * sizeof(FVertex));
                    memcpy((u8*)data + vertexArray.GetCount() * sizeof(FVertex),
                        indexArray.GetData(),
                        indexArray.GetCount() * sizeof(FDrawIndex));
                }
                quadsBuffer[imageIndex]->Unmap();
            }

            if (clipRectArray.GetCount() > clipRectBuffer.GetElementCount())
            {
                u32 totalElementsRequired = Math::Max<u32>(clipRectArray.GetCount(), clipRectBuffer.GetElementCount() + clipRectGrowCount);
                clipRectBuffer.GrowToFit(totalElementsRequired);

                for (int i = 0; i < numFrames; ++i)
                {
                    perObjectSrg->Bind(i, "_ClipRects", clipRectBuffer.GetBuffer(i));
                }
            }

            if (objectDataArray.GetCount() > objectDataBuffer.GetElementCount())
            {
                u32 totalElementsRequired = Math::Max<u32>(objectDataArray.GetCount(), objectDataBuffer.GetElementCount() + objectDataGrowCount);
                objectDataBuffer.GrowToFit(totalElementsRequired);

                for (int i = 0; i < numFrames; ++i)
                {
                    perObjectSrg->Bind(i, "_Objects", objectDataBuffer.GetBuffer(i));
                }
            }

            if (gradientKeyArray.GetCount() > gradientBuffer.GetElementCount())
            {
                u32 totalElementsRequired = Math::Max<u32>(gradientKeyArray.GetCount(), 
                    Math::RoundToInt(gradientBuffer.GetElementCount() * (1 + gradientGrowRatio)));
                gradientBuffer.GrowToFit(totalElementsRequired);

                for (int i = 0; i < numFrames; ++i)
                {
                    perObjectSrg->Bind(i, "_GradientKeys", gradientBuffer.GetBuffer(i));
                }
            }

            if (objectDataArray.GetCount() > 0)
	        {
		        void* data;
            	objectDataBuffer.Map(imageIndex, 0, objectDataArray.GetCount() * sizeof(FObjectData), &data);
	            {
		        	memcpy(data, objectDataArray.GetData(), objectDataArray.GetCount() * sizeof(FObjectData));
	            }
            	objectDataBuffer.Unmap(imageIndex);
	        }

            if (clipRectArray.GetCount() > 0)
            {
                clipRectBuffer.GetBuffer(imageIndex)->UploadData(clipRectArray.GetData(), clipRectArray.GetCount() * sizeof(FClipRect));
            }

            if (gradientKeyArray.GetCount() > 0)
            {
                gradientBuffer.GetBuffer(imageIndex)->UploadData(gradientKeyArray.GetData(), gradientKeyArray.GetCount() * sizeof(FColorStop));
            }

            if (drawDataArray.GetCount() > 0)
            {
                if (drawDataArray.GetCount() > drawDataBuffer.GetElementCount())
                {
                    u32 totalElementsRequired = Math::Max<u32>(drawDataArray.GetCount(), 
                        Math::RoundToInt(drawDataBuffer.GetElementCount() * (1 + drawDataGrowRatio)));
                    drawDataBuffer.GrowToFit(totalElementsRequired);

                    for (int i = 0; i < numFrames; ++i)
                    {
                        perObjectSrg->Bind(i, "_DrawData", drawDataBuffer.GetBuffer(i));
                    }
                }
                
                drawDataBuffer.GetBuffer(imageIndex)->UploadData(drawDataArray.GetData(), drawDataArray.GetCount() * sizeof(FDrawData));
            }

            quadUpdatesRequired[imageIndex] = false;
        }

        perObjectSrg->FlushBindings();

        return drawPacketsPerImage[imageIndex];
    }

    Vec2i FusionRenderer2::GetImageAtlasSize()
    {
        auto app = FusionApplication::Get();

        return Vec2i(1, 1) * app->GetImageAtlas()->GetAtlasSize();
    }

    Vec2i FusionRenderer2::GetFontAtlasSize()
    {
        auto app = FusionApplication::Get();

        return Vec2i(1, 1) * app->GetFontManager()->GetDefaultFontAtlas()->GetAtlasSize();
    }

    u32 FusionRenderer2::GetNumImageAtlasLayers()
    {
        auto app = FusionApplication::Get();

        return app->GetImageAtlas()->GetArrayLayers();
    }

    bool FusionRenderer2::IsRectClipped(const Rect& rect)
    {
        if (!clipStack.IsEmpty())
        {
            ZoneScoped;

            // CPU culling
            
            Vec2 globalTopLeft = coordinateSpaceStack.Last().transform *
                Matrix4x4::Translation(coordinateSpaceStack.Last().translation) *
                Vec4(rect.min.x, rect.min.y, 0, 1);
            Vec2 globalBottomRight = coordinateSpaceStack.Last().transform *
                Matrix4x4::Translation(coordinateSpaceStack.Last().translation) *
                Vec4(rect.max.x, rect.max.y, 0, 1);

            for (int i = clipStack.GetCount() - 1; i >= 0; --i)
            {
                int clipIndex = clipStack[i];

                Matrix4x4 clipTransform = clipRectArray[clipIndex].clipTransform.GetInverse();
                Vec2 clipPos = clipTransform * Vec4(0, 0, 0, 1);
                Vec2 clipBottomRight = clipTransform * Vec4(clipRectArray[clipIndex].size.x, clipRectArray[clipIndex].size.y, 0, 1);

                Rect shapeRect = Rect(globalTopLeft, globalBottomRight);
                Rect clipRect = Rect(clipPos, clipBottomRight);

                if (!shapeRect.Overlaps(clipRect))
                {
                    return true; // Rect is Clipped
                }
            }
        }

        return false;
    }

    FFontMetrics FusionRenderer2::GetFontMetrics(const FFont& font)
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = currentFont.GetFamily();
        int fontSize = currentFont.GetFontSize();

        if (fontSize <= 0)
            fontSize = 12;
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
        if (fontAtlas == nullptr)
            return {};

        return fontAtlas->GetMetrics();
    }

    FFontMetrics FusionRenderer2::GetSDFFontMetrics(const FFont& font)
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = currentFont.GetFamily();
        int fontSize = currentFont.GetFontSize();

        if (fontSize <= 0)
            fontSize = 12;
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        Ref<FSDFFontAtlas> fontAtlas = fontManager->FindSDFFont(fontFamily);
        if (fontAtlas == nullptr)
            return {};

        return fontAtlas->GetMetrics();
    }

    u64 FusionRenderer2::ComputeMemoryFootprint()
    {
	    u64 footprint = Object::ComputeMemoryFootprint();

        footprint += vertexArray.GetCapacity() * sizeof(FVertex);
        footprint += indexArray.GetCapacity() * sizeof(FDrawIndex);
        footprint += path.GetCapacity() * sizeof(Vec2);
        footprint += temporaryPoints.GetCapacity() * sizeof(Vec2);
        footprint += drawCmdList.GetCapacity() * sizeof(FDrawCmd);
        footprint += drawPacketsPerImage[0].GetSize() * sizeof(RHI::DrawPacket) * numFrames;
        footprint += freePackets.GetSize() * sizeof(RHI::DrawPacket);
        footprint += clipStack.GetCapacity() * sizeof(int);
        footprint += clipRectArray.GetCapacity() * sizeof(FClipRect);
        footprint += coordinateSpaceStack.GetCapacity() * sizeof(FCoordinateSpace);
        footprint += opacityStack.GetCapacity() * sizeof(f32);
        footprint += objectDataArray.GetCapacity() * sizeof(FObjectData);
        footprint += gradientKeyArray.GetCapacity() * sizeof(FColorStop);

        footprint += viewConstantsBuffer[0]->GetBufferSize() * numFrames;
        footprint += quadsBuffer[0]->GetBufferSize() * numFrames;
        footprint += objectDataBuffer.GetBuffer(0)->GetBufferSize() * numFrames;
        footprint += clipRectBuffer.GetBuffer(0)->GetBufferSize() * numFrames;
        footprint += drawDataBuffer.GetBuffer(0)->GetBufferSize() * numFrames;

        return footprint;
    }

    void FusionRenderer2::SetDebugName(const CE::Name& name)
    {
		this->debugName = name;
    }

    void FusionRenderer2::Begin()
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        currentBrush = FBrush();
        currentPen = FPen();
        currentFont = FFont(fontManager->GetDefaultFontFamily(), fontManager->GetDefaultFontSize());

        coordinateSpaceStack.RemoveAll();
        drawCmdList.RemoveAll();
        vertexArray.RemoveAll();
        indexArray.RemoveAll();
        PathClear();
        objectDataArray.RemoveAll();
        clipRectArray.RemoveAll();
        clipStack.RemoveAll();
        opacityStack.RemoveAll();
        drawDataArray.RemoveAll();
        gradientKeyArray.RemoveAll();
        indexWritePtr = nullptr;
        vertexWritePtr = nullptr;
        vertexCurrentIdx = 0;

        opacityStack.Insert(1.0f);

        PushChildCoordinateSpace(Matrix4x4::Identity());
    }

    void FusionRenderer2::End()
    {
        ZoneScoped;

        PopChildCoordinateSpace();

        opacityStack.RemoveLast();

        for (int imageIdx = 0; imageIdx < numFrames; ++imageIdx)
        {
            quadUpdatesRequired[imageIdx] = true;

            Array<RHI::DrawPacket*> oldPackets = this->drawPacketsPerImage[imageIdx];
            this->drawPacketsPerImage[imageIdx].Clear();

            for (int i = 0; i < drawCmdList.GetCount(); ++i)
            {
                if (drawCmdList[i].numIndices == 0)
                    continue;

                if (oldPackets.NotEmpty() || freePackets.NotEmpty())
                {
                    RHI::DrawPacket* drawPacket = nullptr;
                    if (oldPackets.NotEmpty())
                    {
                        drawPacket = oldPackets[0];
                        oldPackets.RemoveAt(0);
                    }
                    else
                    {
                        drawPacket = freePackets[0];
                        freePackets.RemoveAt(0);
                    }

                    RHI::DrawIndexedArguments drawArgs{};
                    drawArgs.firstIndex = drawCmdList[i].indexOffset;
                    drawArgs.firstInstance = drawCmdList[i].firstInstance;
                    drawArgs.vertexOffset = drawCmdList[i].vertexOffset;
                    drawArgs.instanceCount = 1;
                    drawArgs.indexCount = drawCmdList[i].numIndices;

                    drawPacket->drawItems[0].arguments = drawArgs;

                    drawPacket->shaderResourceGroups[0] = perObjectSrg;
                    drawPacket->shaderResourceGroups[1] = perViewSrg;
                    drawPacket->shaderResourceGroups[2] = drawCmdList[i].fontSrg;
                    if (drawCmdList[i].textureSrgOverride != nullptr)
                    {
                        drawPacket->shaderResourceGroups[3] = drawCmdList[i].textureSrgOverride;
                    }
                    else
                    {
                        drawPacket->shaderResourceGroups[3] = FusionApplication::Get()->GetImageAtlas()->GetTextureSrg();
                    }

                    memcpy((u8*)drawPacket->rootConstants, &drawCmdList[i].rootConstants, sizeof(FRootConstants));

                    drawPacket->drawItems[0].vertexBufferViews[0] = RHI::VertexBufferView(quadsBuffer[imageIdx],
                                                                                          0,
                                                                                          vertexArray.GetCount() * sizeof(FVertex),
                                                                                          sizeof(FVertex));

                    drawPacket->drawItems[0].indexBufferView[0] = RHI::IndexBufferView(quadsBuffer[imageIdx],
                                                                                       vertexArray.GetCount() * sizeof(FVertex),
                                                                                       indexArray.GetCount() * sizeof(FDrawIndex),
                                                                                       sizeof(FDrawIndex) == 2 ? RHI::IndexFormat::Uint16 : RHI::IndexFormat::Uint32);

                    drawPacket->drawItems[0].pipelineState = fusionShader->GetVariant(0)->GetPipeline(multisampling);

                    this->drawPacketsPerImage[imageIdx].Add(drawPacket);
                }
                else
                {
                    RHI::DrawPacketBuilder builder{};

                    RHI::DrawIndexedArguments drawArgs{};
                    drawArgs.firstIndex = drawCmdList[i].indexOffset;
                    drawArgs.firstInstance = drawCmdList[i].firstInstance;
                    drawArgs.vertexOffset = drawCmdList[i].vertexOffset;
                    drawArgs.instanceCount = 1;
                    drawArgs.indexCount = drawCmdList[i].numIndices;

                    builder.SetDrawArguments(drawArgs);

                    builder.AddShaderResourceGroup(perObjectSrg);
                    builder.AddShaderResourceGroup(perViewSrg);
                    builder.AddShaderResourceGroup(drawCmdList[i].fontSrg);
                    if (drawCmdList[i].textureSrgOverride != nullptr)
                    {
                        builder.AddShaderResourceGroup(drawCmdList[i].textureSrgOverride);
                    }
                    else
                    {
                        builder.AddShaderResourceGroup(FusionApplication::Get()->GetImageAtlas()->GetTextureSrg());
                    }

                    builder.SetRootConstants((u8*)&drawCmdList[i].rootConstants, sizeof(FRootConstants));

                    // UI Item
                    {
                        RHI::DrawPacketBuilder::DrawItemRequest request{};
                        request.vertexBufferViews.Add(RHI::VertexBufferView(quadsBuffer[imageIdx],
                                                                            0,
                                                                            vertexArray.GetCount() * sizeof(FVertex),
                                                                            sizeof(FVertex)));

                        request.indexBufferView = RHI::IndexBufferView(quadsBuffer[imageIdx],
                                                                       vertexArray.GetCount() * sizeof(FVertex),
                                                                       indexArray.GetCount() * sizeof(FDrawIndex),
                                                                       sizeof(FDrawIndex) == 2 ? RHI::IndexFormat::Uint16 : RHI::IndexFormat::Uint32);

                        request.drawItemTag = drawListTag;
                        request.drawFilterMask = RHI::DrawFilterMask::ALL;

                        request.pipelineState = fusionShader->GetVariant(0)->GetPipeline(multisampling);

                        builder.AddDrawItem(request);
                    }

                    RHI::DrawPacket* drawPacket = builder.Build();
                    this->drawPacketsPerImage[imageIdx].Add(drawPacket);
                }
            }

            if (oldPackets.NotEmpty())
            {
                freePackets.AddRange(oldPackets);
                oldPackets.Clear();
            }
        }
    }

    void FusionRenderer2::PushChildCoordinateSpace(const Matrix4x4& transform)
    {
        ZoneScoped;

        if (coordinateSpaceStack.IsEmpty())
        {
            coordinateSpaceStack.Insert(FCoordinateSpace{ 
                .transform = transform, .translation = Vec3(), .isTranslationOnly = false, .indexInObjectArray = (int)objectDataArray.GetCount() });
        }
        else
        {
            if (coordinateSpaceStack.Last().isTranslationOnly)
            {
                coordinateSpaceStack.Insert(FCoordinateSpace{
	                .transform = coordinateSpaceStack.Last().transform * Matrix4x4::Translation(coordinateSpaceStack.Last().translation) * transform,
	                .translation = Vec3(),
	                .isTranslationOnly = false,
					.indexInObjectArray = (int)objectDataArray.GetCount() });
            }
            else
            {
                coordinateSpaceStack.Insert(FCoordinateSpace{
	                .transform = coordinateSpaceStack.Last().transform * transform,
	                .translation = Vec3(),
	                .isTranslationOnly = false,
                    .indexInObjectArray = (int)objectDataArray.GetCount() });
            }
        }

        objectDataArray.Insert(FObjectData{ .transform = coordinateSpaceStack.Last().transform });

        AddDrawCmd();
    }

    void FusionRenderer2::PushChildCoordinateSpace(Vec2 translation)
    {
        ZoneScoped;

        if (coordinateSpaceStack.Last().isTranslationOnly)
        {
            coordinateSpaceStack.Insert(FCoordinateSpace{
                .transform = coordinateSpaceStack.Last().transform,
                .translation = coordinateSpaceStack.Last().translation + translation,
                .isTranslationOnly = true,
				.indexInObjectArray = coordinateSpaceStack.Last().indexInObjectArray });
        }
        else
        {
            coordinateSpaceStack.Insert(FCoordinateSpace{
                .transform = coordinateSpaceStack.Last().transform,
                .translation = translation,
                .isTranslationOnly = true,
                .indexInObjectArray = coordinateSpaceStack.Last().indexInObjectArray });
        }
    }

    void FusionRenderer2::PopChildCoordinateSpace()
    {
        ZoneScoped;

        if (!coordinateSpaceStack.IsEmpty())
        {
            bool isTranslation = coordinateSpaceStack.Last().isTranslationOnly;

            coordinateSpaceStack.RemoveLast();

            if (!isTranslation && !coordinateSpaceStack.IsEmpty())
            {
                AddDrawCmd();
            }
        }
    }

    Matrix4x4 FusionRenderer2::GetTopCoordinateSpace()
    {
        if (coordinateSpaceStack.Last().isTranslationOnly)
        {
            return coordinateSpaceStack.Last().transform * Matrix4x4::Translation(coordinateSpaceStack.Last().translation);
        }

        return coordinateSpaceStack.Last().transform;
    }

    void FusionRenderer2::PushClipRect(const Matrix4x4& clipTransform, Vec2 rectSize)
    {
        ZoneScoped;

        Matrix4x4 inverse = (coordinateSpaceStack.Last().transform * Matrix4x4::Translation(coordinateSpaceStack.Last().translation) * clipTransform).GetInverse();

        // Clipping is done via SDF functions, which require you to inverse the transformations applied
        clipRectArray.Insert(FClipRect{
            .clipTransform = inverse,
            .size = rectSize
        });

        clipStack.Insert((int)clipRectArray.GetCount() - 1);

        AddDrawCmd();
    }

    void FusionRenderer2::PopClipRect()
    {
        ZoneScoped;

        if (clipStack.IsEmpty())
            return;

        clipStack.RemoveLast();

        AddDrawCmd();
    }

    void FusionRenderer2::PushOpacity(f32 opacity)
    {
        opacityStack.Insert(opacity);
    }

    void FusionRenderer2::PopOpacity()
    {
        opacityStack.RemoveLast();
    }

    void FusionRenderer2::SetPen(const FPen& pen)
    {
        this->currentPen = pen;
    }

    void FusionRenderer2::SetBrush(const FBrush& brush)
    {
        this->currentBrush = brush;
    }

    void FusionRenderer2::SetFont(const FFont& font)
    {
        bool familyChanged = this->currentFont.GetFamily() != font.GetFamily();
        this->currentFont = font;

        if (familyChanged)
        {
            AddDrawCmd();
        }
    }

    void FusionRenderer2::PathClear()
    {
        path.RemoveAll();

        pathMax = Vec2(NumericLimits<f32>::Min(), NumericLimits<f32>::Min());
        pathMin = Vec2(NumericLimits<f32>::Max(), NumericLimits<f32>::Max());
    }

    void FusionRenderer2::PathLineTo(const Vec2& point)
    {
        PathInsert(point);
    }

    void FusionRenderer2::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle)
    {
        if (radius < 0.5f)
        {
            PathInsert(center);
	        return;
        }

        const float arcLength = Math::Abs(endAngle - startAngle);
        const int circleSegmentCount = CalculateNumCircleSegments(radius);
        const int arcSegmentCount = Math::Max(Math::RoundToInt(ceilf(circleSegmentCount * arcLength / (Math::PI * 2.0f))), 
            Math::RoundToInt(2.0f * Math::PI / arcLength));

        PathArcTo(center, radius, startAngle, endAngle, arcSegmentCount);
    }

    void FusionRenderer2::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, int numSegments)
    {
        if (radius < 0.5f)
        {
            PathInsert(center);
	        return;
        }

        for (int i = 0; i <= numSegments; ++i)
        {
            const float angle = startAngle + ((float)i / (float)numSegments) * (endAngle - startAngle);
            const float degrees = Math::ToDegrees(angle);
            PathInsert(Vec2(center.x + Math::Cos(angle) * radius, center.y + Math::Sin(angle) * radius));
        }
    }

    void FusionRenderer2::PathArcToFast(const Vec2& center, float radius, int angleMinOf12, int angleMaxOf12)
    {
        if (radius < 0.5f)
        {
            PathInsert(center);
            return;
        }
        PathArcToFastInternal(center, radius, angleMinOf12 * ArcFastTableSize / 12, angleMaxOf12 * ArcFastTableSize / 12, 0);
    }

    void FusionRenderer2::PathRect(const Rect& rect, const Vec4& cornerRadius)
    {
        const Vec2& min = rect.min;
        const Vec2& max = rect.max;

        if (cornerRadius.GetMax() < 0.5f)
        {
            PathLineTo(min);
            PathLineTo(Vec2(max.x, min.y));
            PathLineTo(max);
            PathLineTo(Vec2(min.x, max.y));
        }
        else
        {
            PathArcToFast(Vec2(min.x + cornerRadius.topLeft, min.y + cornerRadius.topLeft), cornerRadius.topLeft,
                6, 9);
            PathArcToFast(Vec2(max.x - cornerRadius.topRight, min.y + cornerRadius.topRight), cornerRadius.topRight,
                9, 12);
            PathArcToFast(Vec2(max.x - cornerRadius.bottomRight, max.y - cornerRadius.bottomRight), cornerRadius.bottomRight,
                0, 3);
            PathArcToFast(Vec2(min.x + cornerRadius.bottomLeft, max.y - cornerRadius.bottomLeft), cornerRadius.bottomLeft,
                3, 6);
        }
    }

    // Credit: Dear ImGui

    static Vec2 ImBezierCubicCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, float t)
    {
        float u = 1.0f - t;
        float w1 = u * u * u;
        float w2 = 3 * u * u * t;
        float w3 = 3 * u * t * t;
        float w4 = t * t * t;
        return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
    }

    static Vec2 ImBezierQuadraticCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, float t)
    {
        float u = 1.0f - t;
        float w1 = u * u;
        float w2 = 2 * u * t;
        float w3 = t * t;
        return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y);
    }

    void PathBezierCubicCurveToCasteljau(FusionRenderer2* renderer,
        float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
    {
        float dx = x4 - x1;
        float dy = y4 - y1;
        float d2 = (x2 - x4) * dy - (y2 - y4) * dx;
        float d3 = (x3 - x4) * dy - (y3 - y4) * dx;
        d2 = (d2 >= 0) ? d2 : -d2;
        d3 = (d3 >= 0) ? d3 : -d3;
        if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
        {
            renderer->PathInsert(Vec2(x4, y4));
        }
        else if (level < 10)
        {
            float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
            float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
            float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
            PathBezierCubicCurveToCasteljau(renderer, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
            PathBezierCubicCurveToCasteljau(renderer, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
        }
    }

    void PathBezierQuadraticCurveToCasteljau(FusionRenderer2* renderer,
        float x1, float y1, float x2, float y2, float x3, float y3, float tess_tol, int level)
    {
        float dx = x3 - x1, dy = y3 - y1;
        float det = (x2 - x3) * dy - (y2 - y3) * dx;
        if (det * det * 4.0f < tess_tol * (dx * dx + dy * dy))
        {
            renderer->PathInsert(Vec2(x3, y3));
        }
        else if (level < 10)
        {
            float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            PathBezierQuadraticCurveToCasteljau(renderer, x1, y1, x12, y12, x123, y123, tess_tol, level + 1);
            PathBezierQuadraticCurveToCasteljau(renderer, x123, y123, x23, y23, x3, y3, tess_tol, level + 1);
        }
    }

    void FusionRenderer2::PathBezierCubicCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, int numSegments)
    {
        Vec2 p1 = path.Last();
        if (numSegments == 0)
        {
            // Auto-tessellated
            PathBezierCubicCurveToCasteljau(this, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, curveTessellationTolerance, 0);
        }
        else
        {
            float totalSteps = 1.0f / (float)numSegments;
            for (int i = 1; i <= numSegments; i++)
            {
                PathInsert(ImBezierCubicCalc(p1, p2, p3, p4, totalSteps * i));
            }
        }
    }

    void FusionRenderer2::PathQuadraticCubicCurveTo(const Vec2& p2, const Vec2& p3, int numSegments)
    {
        Vec2 p1 = path.Last();
        if (numSegments == 0)
        {
            // Auto-tessellated
            PathBezierQuadraticCurveToCasteljau(this, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, curveTessellationTolerance, 0);
        }
        else
        {
            float totalSteps = 1.0f / (float)numSegments;
            for (int i = 1; i <= numSegments; i++)
            {
                PathInsert(ImBezierQuadraticCalc(p1, p2, p3, totalSteps * i));
            }
        }
    }

    bool FusionRenderer2::PathFill(bool antiAliased)
    {
        if (IsRectClipped(Rect(pathMin, pathMax)))
        {
            PathClear();
            return false;
        }

        if (path.IsEmpty())
        {
            return true;
        }

        Rect minMax = Rect(pathMin, pathMax);

        switch (currentBrush.GetBrushStyle())
        {
        case FBrushStyle::None:
	        break;
        case FBrushStyle::SolidFill:
            AddConvexPolyFilled(path.GetData(), path.GetCount(), antiAliased);
	        break;
        case FBrushStyle::Image:
            AddConvexPolyFilled(path.GetData(), path.GetCount(), antiAliased, &minMax);
	        break;
        case FBrushStyle::Gradient:
            AddConvexPolyFilled(path.GetData(), path.GetCount(), antiAliased, &minMax);
	        break;
        }

        PathClear();

        return true;
    }

    bool FusionRenderer2::PathStroke(bool closed, bool antiAliased)
    {
        if (IsRectClipped(Rect(pathMin, pathMax)))
        {
            PathClear();
            return false;
        }

        // TODO: Implement dotted and dashed lines

        f32 thickness = currentPen.GetThickness();

        switch (currentPen.GetStyle())
        {
        case FPenStyle::None:
	        break;
        case FPenStyle::SolidLine:
            AddPolyLine(path.GetData(), (int)path.GetCount(), thickness, closed, antiAliased);
	        break;
        case FPenStyle::DashedLine:
	        {
		        const f32 dashLength = currentPen.GetDashLength();
                const f32 dashSeparation = dashLength / 2;
                f32 distanceWithoutPainting = dashLength + 1;

                for (int i = 0; i < path.GetCount(); ++i)
                {
                    Vec2 p0 = path[i];
                    if (!closed && i == (int)path.GetCount() - 1)
                        break;
                    Vec2 p1 = path[(i + 1) % path.GetCount()];

                    if (Vec2::SqrDistance(p0, p1) > dashLength * dashLength)
                    {
                        // Split the line segment into individual dotted and non-dotted segments
                        int numSegments = (int)(Vec2::Distance(p0, p1) / dashLength);

                        for (int j = 0; j < numSegments; ++j)
                        {
                            if (j % 2 != 0) // Skip odd segments
                                continue;

                            distanceWithoutPainting = 0;

                            f32 curStep = (f32)j / numSegments;
                            f32 nextStep = (f32)(j + 1) / numSegments;
                            Vec2 curPoint = p0 + (p1 - p0) * curStep;
                            Vec2 nextPoint = p0 + (p1 - p0) * nextStep;

                            Vec2 points[2] = { curPoint, nextPoint };

                            AddPolyLine(points, 2, thickness, false, antiAliased);
                        }
                    }
                    else // Distance between the points is less than the individual dot length, so just draw a normal solid line.
                    {
                        if (distanceWithoutPainting >= dashLength)
                        {
                            Vec2 points[2] = { p0, p1 };

                            AddPolyLine(points, 2, thickness, false, antiAliased);

                            distanceWithoutPainting = 0;
                        }
                        else
                        {
                            distanceWithoutPainting += Vec2::Distance(p0, p1);
                        }
                    }
                }
	        }
	        break;
        case FPenStyle::DottedLine:
	        {
		        constexpr f32 dottedLength = 1;
                f32 distanceWithoutPainting = dottedLength + 1;

				for (int i = 0; i < path.GetCount(); ++i)
				{
                    Vec2 p0 = path[i];
                    if (!closed && i == (int)path.GetCount() - 1)
                        break;
                    Vec2 p1 = path[(i + 1) % path.GetCount()];

                    if (Vec2::SqrDistance(p0, p1) > dottedLength * dottedLength)
                    {
                        f32 dist = Vec2::Distance(p0, p1);
                        // Split the line segment into individual dotted and non-dotted segments
                        int numSegments = (int)(dist / dottedLength);

                        for (int j = 0; j < numSegments; ++j)
                        {
                            if (j % 2 != 0) // Skip odd segments
                                continue;

                            distanceWithoutPainting = 0;

                            f32 curStep = (f32)j / numSegments;
                            f32 nextStep = (f32)(j + 1) / numSegments;
                            Vec2 curPoint = p0 + (p1 - p0) * curStep;
                            Vec2 nextPoint = p0 + (p1 - p0) * nextStep;

                            Vec2 points[2] = { curPoint, nextPoint };

                            AddPolyLine(points, 2, thickness, false, antiAliased);
                        }
                    }
                    else // Distance between the points is less than the individual dot length, so just draw a normal solid line.
                    {
                        if (distanceWithoutPainting >= dottedLength)
                        {
                            Vec2 points[2] = { p0, p1 };

                            AddPolyLine(points, 2, thickness, false, antiAliased);

                            distanceWithoutPainting = 0;
                        }
                        else
                        {
                            distanceWithoutPainting += Vec2::Distance(p0, p1);
                        }
                    }
				}
	        }
	        break;
        }

        PathClear();

        return true;
    }

    bool FusionRenderer2::PathFillStroke(bool closed, bool antiAliased)
    {
        if (IsRectClipped(Rect(pathMin, pathMax)))
        {
            PathClear();
            return false;
        }

        Rect minMax = Rect(pathMin, pathMax);

        switch (currentBrush.GetBrushStyle())
        {
        case FBrushStyle::None:
	        break;
        case FBrushStyle::SolidFill:
            AddConvexPolyFilled(path.GetData(), (int)path.GetCount(), antiAliased);
	        break;
        case FBrushStyle::Image:
            AddConvexPolyFilled(path.GetData(), (int)path.GetCount(), antiAliased, &minMax);
	        break;
        case FBrushStyle::Gradient:
	        break;
        }

        AddPolyLine(path.GetData(), (int)path.GetCount(), currentPen.GetThickness(), closed, antiAliased);

        PathClear();

        return true;
    }

    bool FusionRenderer2::FillRect(const Rect& rect, const Vec4& cornerRadius, bool antiAliased)
    {
        return AddRectFilled(rect, cornerRadius, antiAliased);
    }

    bool FusionRenderer2::StrokeRect(const Rect& rect, const Vec4& cornerRadius, bool antiAliased)
    {
        return AddRect(rect, cornerRadius, antiAliased);
    }

    bool FusionRenderer2::FillCircle(const Vec2& center, f32 radius, bool antiAliased)
    {
        return AddCircleFilled(center, radius, 0, antiAliased);
    }

    bool FusionRenderer2::StrokeCircle(const Vec2& center, f32 radius, bool antiAliased)
    {
        return AddCircle(center, radius, 0, antiAliased);
    }

    Vec2 FusionRenderer2::DrawText(const String& text, Vec2 textPos, Vec2 size, FWordWrap wordWrap)
    {
        thread_local Array<Rect> quads{};
        const bool isFixedSize = !Math::ApproxEquals(size.x, 0.0f) && !Math::ApproxEquals(size.y, 0.0f);

        if (isFixedSize && IsRectClipped(Rect::FromSize(textPos, size)))
        {
            return Vec2();
        }

        quads.Reserve(text.GetLength());

        Vec2 finalSize = CalculateTextQuads(quads, text, currentFont, size.width, wordWrap);

        if (!isFixedSize && IsRectClipped(Rect::FromSize(textPos, finalSize)))
        {
            return Vec2();
        }

        DrawTextInternal(quads.GetData(), text.GetData(), text.GetLength(), currentFont, textPos);
        return finalSize;
    }

    Vec2 FusionRenderer2::DrawSDFText(const String& text, Vec2 textPos, Vec2 size, FWordWrap wordWrap)
    {
        thread_local Array<Rect> quads{};
        const bool isFixedSize = !Math::ApproxEquals(size.x, 0.0f) && !Math::ApproxEquals(size.y, 0.0f);

        if (isFixedSize && IsRectClipped(Rect::FromSize(textPos, size)))
        {
            return Vec2();
        }

        quads.Reserve(text.GetLength());

        Vec2 finalSize = CalculateSDFTextQuads(quads, text, currentFont, size.width, wordWrap);

        if (!isFixedSize && IsRectClipped(Rect::FromSize(textPos, finalSize)))
        {
            return Vec2();
        }

        DrawSDFTextInternal(quads.GetData(), text.GetData(), text.GetLength(), currentFont, textPos);
        return finalSize;
    }

    void FusionRenderer2::DrawViewport(const Rect& rect, FViewport* viewport)
    {
        AddDrawCmd();

        PrimReserve(4, 6);

        u32 color = Color::White().ToU32();

        drawCmdList.Last().textureSrgOverride = viewport->GetTextureSrg();

        Vec2 offset = Vec2();
        if (coordinateSpaceStack.Last().isTranslationOnly)
        {
            offset = coordinateSpaceStack.Last().translation;
        }

        Rect quad = rect.Translate(offset);

        Vec2 topLeft = quad.min;
        Vec2 topRight = Vec2(quad.max.x, quad.min.y);
        Vec2 bottomRight = Vec2(quad.max.x, quad.max.y);
        Vec2 bottomLeft = Vec2(quad.min.x, quad.max.y);

        Vec2 topLeftUV = Vec2(0, 0);
        Vec2 topRightUV = Vec2(1, 0);
        Vec2 bottomRightUV = Vec2(1, 1);
        Vec2 bottomLeftUV = Vec2(0, 1);

        FDrawIndex idx = vertexCurrentIdx;
        indexWritePtr[0] = idx; indexWritePtr[1] = (idx + 1); indexWritePtr[2] = (idx + 2);
        indexWritePtr[3] = idx; indexWritePtr[4] = (idx + 2); indexWritePtr[5] = (idx + 3);

        vertexWritePtr[0].position = topLeft; vertexWritePtr[0].color = color; vertexWritePtr[0].uv = topLeftUV; vertexWritePtr[0].drawType = DRAW_Viewport;
        vertexWritePtr[1].position = topRight; vertexWritePtr[1].color = color; vertexWritePtr[1].uv = topRightUV; vertexWritePtr[1].drawType = DRAW_Viewport;
        vertexWritePtr[2].position = bottomRight; vertexWritePtr[2].color = color; vertexWritePtr[2].uv = bottomRightUV; vertexWritePtr[2].drawType = DRAW_Viewport;
        vertexWritePtr[3].position = bottomLeft; vertexWritePtr[3].color = color; vertexWritePtr[3].uv = bottomLeftUV; vertexWritePtr[3].drawType = DRAW_Viewport;
        vertexWritePtr += 4;
        vertexCurrentIdx += 4;
        indexWritePtr += 6;

        drawCmdList.Last().numIndices += 6;

        AddDrawCmd();
    }

    void FusionRenderer2::DrawImageAtlas(const Rect& quad, int layerIndex)
    {
        PrimReserve(4, 6);

        u32 color = Color::White().ToU32();

        PrimRect(quad, color, nullptr, DRAW_TextureAtlas, layerIndex);
    }

    void FusionRenderer2::DrawFontAtlas(const Rect& rect, const Ref<FFontAtlas>& fontAtlas, int layerIndex)
    {
        PrimReserve(4, 6);

        u32 color = Color::White().ToU32();

        PrimRect(rect, color, nullptr, DRAW_FontAtlas, layerIndex);
    }

    Vec2 FusionRenderer2::CalculateTextQuads(Array<Rect>& outQuads, const String& text, const FFont& font,
                                             f32 width, FWordWrap wordWrap)
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max(fontSize, MinFontSize);

        const bool isFixedWidth = width > 0.1f;

        FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
        if (fontAtlas == nullptr)
            return Vec2();
        
        const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
        const f32 fontDpiScaling = dpi / 72.0f;
        const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
        const f32 metricsScaling = fontDpiScaling / systemDpiScaling;
#if PLATFORM_MAC
        fontSize *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

        const FFontMetrics& metrics = fontAtlas->GetMetrics();

        const float startY = metrics.ascender * (f32)fontSize * metricsScaling;
        constexpr float startX = 0;

        float maxX = startX;
        float maxY = startY;

        Vec3 curPos = Vec3(startX, startY, 0);

        int totalCharacters = 0;
        int breakCharIdx = -1;
        int idx = 0;

        outQuads.Resize(text.GetLength());
        int numQuads = outQuads.GetSize();

        for (int i = 0; i < text.GetLength(); ++i)
        {
            char c = text[i];

            if (c == ' ' || c == '-' || c == '\\' || c == '/')
            {
                breakCharIdx = i;
            }

            if (c == '\n')
            {
                breakCharIdx = -1;
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;
                outQuads[i] = Rect();
                continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());

            const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphAdvance = (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            
            if (isFixedWidth && (curPos.x + glyphAdvance > width + 0.1f) && wordWrap != FWordWrap::NoWrap)
            {
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;

                // Go through previous characters and bring them to this new-line
                if (breakCharIdx >= 0)
                {
                    for (int j = breakCharIdx + 1; j < i; j++)
                    {
                        char prevChar = text[j];
                        FFontGlyphInfo prevGlyph = fontAtlas->FindOrAddGlyph(prevChar, fontSize, currentFont.IsBold(), currentFont.IsItalic());
                        f32 atlasFontSize = prevGlyph.fontSize;

                        outQuads[j] = Rect::FromSize(
                            curPos.x + (f32)prevGlyph.xOffset * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling,
                            curPos.y - (f32)prevGlyph.yOffset * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling,
                            (f32)prevGlyph.GetWidth() * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling,
                            (f32)prevGlyph.GetHeight() * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling);

                        curPos.x += (f32)prevGlyph.advance * fontSize / atlasFontSize / systemDpiScaling;
                    }
                    breakCharIdx = -1;

                    curPos.x += (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
                }
                else if (wordWrap == FWordWrap::BreakWord)
                {
                    breakCharIdx = -1;
                    curPos.x = startX;
                    curPos.y += metrics.lineHeight * fontSize * metricsScaling;
                }
            }

            // Apply glyph offsets
            outQuads[i] = Rect::FromSize(
                curPos.x + (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling,
                curPos.y - (f32)glyph.yOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling,
                glyphWidth, glyphHeight);

            curPos.x += glyphAdvance;

            maxX = Math::Max(curPos.x, maxX);
            maxY = Math::Max(curPos.y + metrics.lineHeight * (f32)fontSize * metricsScaling, maxY);

            totalCharacters++;
        }

        Vec2 finalSize = Vec2(maxX - startX, maxY - startY);
        if (isFixedWidth)
            finalSize.width = width;

        return finalSize;
    }

    Vec2 FusionRenderer2::CalculateSDFTextQuads(Array<Rect>& outQuads, const String& text, const FFont& font, f32 width, FWordWrap wordWrap)
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max(fontSize, MinFontSize);

        const bool isFixedWidth = width > 0.1f;

        Ref<FSDFFontAtlas> fontAtlas = fontManager->FindSDFFont(fontFamily);
        if (fontAtlas == nullptr)
            return Vec2();

        const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
        const f32 fontDpiScaling = dpi / 72.0f;
        const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
        const f32 metricsScaling = fontDpiScaling / systemDpiScaling;
#if PLATFORM_MAC
        fontSize *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

        constexpr int glyphPadding = 4;

        const FFontMetrics& metrics = fontAtlas->GetMetrics();

        const float startY = metrics.ascender * (f32)fontSize * metricsScaling;
        constexpr float startX = 0;

        float maxX = startX;
        float maxY = startY;

        Vec3 curPos = Vec3(startX, startY, 0);

        int totalCharacters = 0;
        int breakCharIdx = -1;
        int idx = 0;

        outQuads.Resize(text.GetLength());
        int numQuads = outQuads.GetSize();

        for (int i = 0; i < text.GetLength(); ++i)
        {
            char c = text[i];

            if (c == ' ' || c == '-' || c == '\\' || c == '/')
            {
                breakCharIdx = i;
            }

            if (c == '\n')
            {
                breakCharIdx = -1;
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;
                outQuads[i] = Rect();
                continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());

            const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphAdvance = (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            if (isFixedWidth && (curPos.x + glyphAdvance > width + 0.1f) && wordWrap != FWordWrap::NoWrap)
            {
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;

                // Go through previous characters and bring them to this new-line
                if (breakCharIdx >= 0)
                {
                    for (int j = breakCharIdx + 1; j < i; j++)
                    {
                        char prevChar = text[j];
                        FFontGlyphInfo prevGlyph = fontAtlas->FindOrAddGlyph(prevChar, fontSize, currentFont.IsBold(), currentFont.IsItalic());
                        f32 atlasFontSize = prevGlyph.fontSize;

                        outQuads[j] = Rect::FromSize(
                            curPos.x + (f32)prevGlyph.xOffset * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling,
                            curPos.y - (f32)prevGlyph.yOffset * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling,
                            (f32)prevGlyph.GetWidth() * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling,
                            (f32)prevGlyph.GetHeight() * (f32)fontSize / (f32)prevGlyph.fontSize / systemDpiScaling);

                        outQuads[j].min -= Vec2(1, 1) * glyphPadding * fontSize / atlasFontSize / 4;
                        outQuads[j].max += Vec2(1, 1) * glyphPadding * fontSize / atlasFontSize / 4;

                        curPos.x += (f32)prevGlyph.advance * fontSize / atlasFontSize / systemDpiScaling;
                    }
                    breakCharIdx = -1;

                    curPos.x += (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
                }
                else if (wordWrap == FWordWrap::BreakWord)
                {
                    breakCharIdx = -1;
                    curPos.x = startX;
                    curPos.y += metrics.lineHeight * fontSize * metricsScaling;
                }
            }

            // Apply glyph offsets
            outQuads[i] = Rect::FromSize(
                curPos.x + (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling,
                curPos.y - (f32)glyph.yOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling,
                glyphWidth, glyphHeight);

            outQuads[i].min -= Vec2(1, 1) * glyphPadding * fontSize / glyph.fontSize / 4;
            outQuads[i].max += Vec2(1, 1) * glyphPadding * fontSize / glyph.fontSize / 4;

            curPos.x += glyphAdvance;

            maxX = Math::Max(curPos.x, maxX);
            maxY = Math::Max(curPos.y + metrics.lineHeight * (f32)fontSize * metricsScaling, maxY);

            totalCharacters++;
        }

        Vec2 finalSize = Vec2(maxX - startX, maxY - startY);
        if (isFixedWidth)
            finalSize.width = width;

        return finalSize;
    }

    void FusionRenderer2::CalculateUnderlinePositions(Array<Rect>& outLines, const String& text, const FFont& font, f32 width,
                                                      FWordWrap wordWrap)
    {
        ZoneScoped;

        if (text.IsEmpty())
            return;

        outLines.Clear();

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max(fontSize, MinFontSize);

        const bool isFixedWidth = width > 0.1f;

        FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
        if (fontAtlas == nullptr)
            return;

        const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
        const f32 fontDpiScaling = dpi / 72.0f;
        const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
        const f32 metricsScaling = fontDpiScaling / systemDpiScaling;

        const FFontMetrics& metrics = fontAtlas->GetMetrics();

        const float startY = metrics.ascender * fontSize * metricsScaling;
        constexpr float startX = 0;

        float maxX = startX;
        float maxY = startY;

        Vec3 curPos = Vec3(startX, startY, 0);

        int totalCharacters = 0;
        int breakCharIdx = -1;
        int idx = 0;

        bool newLine = true;
        Rect curRect;

        for (int i = 0; i < text.GetLength(); ++i)
        {
            char c = text[i];

            if (c == ' ' || c == '-' || c == '\\' || c == '/')
            {
                breakCharIdx = i;
            }

            if (c == '\n')
            {
                breakCharIdx = -1;
                if (!newLine)
                {
                    outLines.Add(curRect);
                }
                newLine = true;
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;
                curRect.min = curPos;
                continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());

            const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphAdvance = (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            if (isFixedWidth && (curPos.x + glyphAdvance > width + 0.1f) && wordWrap != FWordWrap::NoWrap)
            {
                // TODO: Implement line breaking for underline positions
            }

            if (newLine)
            {
                newLine = false;
                curRect.min = curPos;
            }

            curPos.x += (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            curRect.max.x = curPos.x;
            curRect.max.y = curRect.min.y;

            maxX = Math::Max(curPos.x, maxX);
            maxY = Math::Max(curPos.y + metrics.lineHeight * (f32)fontSize * metricsScaling, maxY);

            totalCharacters++;
        }

        if (!newLine && text.NotEmpty())
        {
            outLines.Add(curRect);
        }
    }

    Vec2 FusionRenderer2::CalculateCharacterOffsets(Array<Vec2>& outOffsets, const String& text, const FFont& font,
                                                    f32 width, FWordWrap wordWrap)
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max<f32>(fontSize, MinFontSize);

        const bool isFixedWidth = width > 0.1f;

        FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
        if (fontAtlas == nullptr)
            return Vec2();

        const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
        const f32 fontDpiScaling = dpi / 72.0f;
        const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
        const f32 metricsScaling = fontDpiScaling / systemDpiScaling;
#if PLATFORM_MAC
        fontSize *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

        const FFontMetrics& metrics = fontAtlas->GetMetrics();

        const float startY = metrics.ascender * (f32)fontSize * metricsScaling;
        constexpr float startX = 0;

        float maxX = startX;
        float maxY = startY;

        Vec3 curPos = Vec3(startX, startY, 0);

        int totalCharacters = 0;
        int breakCharIdx = -1;
        int idx = 0;

        outOffsets.Resize(text.GetLength());

        for (int i = 0; i < text.GetLength(); ++i)
        {
            char c = text[i];

            if (c == ' ' || c == '-' || c == '\\' || c == '/')
            {
                breakCharIdx = i;
            }

            if (c == '\n')
            {
                breakCharIdx = -1;
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;
                outOffsets[i] = Vec2();
                continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());

            const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphAdvance = (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            if (isFixedWidth && (curPos.x + glyphAdvance > width) && wordWrap != FWordWrap::NoWrap)
            {
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;

                // Go through previous characters and bring them to this new-line
                if (breakCharIdx >= 0)
                {
                    for (int j = breakCharIdx + 1; j < i; j++)
                    {
                        char prevChar = text[j];
                        FFontGlyphInfo prevGlyph = fontAtlas->FindOrAddGlyph(prevChar, fontSize, currentFont.IsBold(), currentFont.IsItalic());
                        f32 atlasFontSize = prevGlyph.fontSize;

                        outOffsets[j] = Vec2(curPos.x, curPos.x + (f32)prevGlyph.advance * fontSize / atlasFontSize / systemDpiScaling);

                        curPos.x += (f32)prevGlyph.advance * fontSize / atlasFontSize / systemDpiScaling;
                    }
                    breakCharIdx = -1;
                }
                else if (wordWrap == FWordWrap::BreakWord)
                {
                    breakCharIdx = -1;
                    curPos.x = startX;
                    curPos.y += metrics.lineHeight * fontSize * metricsScaling;
                }
            }

            // Apply glyph offsets
            outOffsets[i] = Vec2(
                curPos.x + (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling,
                curPos.x + (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling + 
                (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling);

            curPos.x += (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            maxX = Math::Max(curPos.x, maxX);
            maxY = Math::Max(curPos.y + metrics.lineHeight * (f32)fontSize * metricsScaling, maxY);

            totalCharacters++;
        }

        Vec2 finalSize = Vec2(maxX - startX, maxY - startY);
        if (isFixedWidth)
            finalSize.width = width;

        return finalSize;
    }

    Vec2 FusionRenderer2::CalculateSDFCharacterOffsets(Array<Vec2>& outOffsets, const String& text, const FFont& font,
	    f32 width, FWordWrap wordWrap)
    {
        ZoneScoped;

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max<f32>(fontSize, MinFontSize);

        const bool isFixedWidth = width > 0.1f;

        Ref<FSDFFontAtlas> fontAtlas = fontManager->FindSDFFont(fontFamily);
        if (fontAtlas == nullptr)
            return Vec2();

        const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
        const f32 fontDpiScaling = dpi / 72.0f;
        const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
        const f32 metricsScaling = fontDpiScaling / systemDpiScaling;
#if PLATFORM_MAC
        fontSize *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

        const FFontMetrics& metrics = fontAtlas->GetMetrics();

        const float startY = metrics.ascender * (f32)fontSize * metricsScaling;
        constexpr float startX = 0;

        float maxX = startX;
        float maxY = startY;

        Vec3 curPos = Vec3(startX, startY, 0);

        int totalCharacters = 0;
        int breakCharIdx = -1;
        int idx = 0;

        outOffsets.Resize(text.GetLength());

        for (int i = 0; i < text.GetLength(); ++i)
        {
            char c = text[i];

            if (c == ' ' || c == '-' || c == '\\' || c == '/')
            {
                breakCharIdx = i;
            }

            if (c == '\n')
            {
                breakCharIdx = -1;
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;
                outOffsets[i] = Vec2();
                continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());

            const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;
            const float glyphAdvance = (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            if (isFixedWidth && (curPos.x + glyphAdvance > width) && wordWrap != FWordWrap::NoWrap)
            {
                curPos.x = startX;
                curPos.y += metrics.lineHeight * (f32)fontSize * metricsScaling;

                // Go through previous characters and bring them to this new-line
                if (breakCharIdx >= 0)
                {
                    for (int j = breakCharIdx + 1; j < i; j++)
                    {
                        char prevChar = text[j];
                        FFontGlyphInfo prevGlyph = fontAtlas->FindOrAddGlyph(prevChar, fontSize, currentFont.IsBold(), currentFont.IsItalic());
                        f32 atlasFontSize = prevGlyph.fontSize;

                        outOffsets[j] = Vec2(curPos.x, curPos.x + (f32)prevGlyph.advance * fontSize / atlasFontSize / systemDpiScaling);

                        curPos.x += (f32)prevGlyph.advance * fontSize / atlasFontSize / systemDpiScaling;
                    }
                    breakCharIdx = -1;
                }
                else if (wordWrap == FWordWrap::BreakWord)
                {
                    breakCharIdx = -1;
                    curPos.x = startX;
                    curPos.y += metrics.lineHeight * fontSize * metricsScaling;
                }
            }

            // Apply glyph offsets
            outOffsets[i] = Vec2(
                curPos.x + (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling,
                curPos.x + (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling +
                (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling);

            curPos.x += (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize / systemDpiScaling;

            maxX = Math::Max(curPos.x, maxX);
            maxY = Math::Max(curPos.y + metrics.lineHeight * (f32)fontSize * metricsScaling, maxY);

            totalCharacters++;
        }

        Vec2 finalSize = Vec2(maxX - startX, maxY - startY);
        if (isFixedWidth)
            finalSize.width = width;

        return finalSize;
    }

    void FusionRenderer2::DrawTextInternal(const Rect* quads, char* text, int length, const FFont& font, Vec2 textPos)
    {
        thread_local HashSet<char> ignoreCharacters = { ' ', '\n', '\r', '\t', '\0' };

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max<f32>(fontSize, MinFontSize);

#if PLATFORM_MAC
        fontSize *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

        FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
        if (fontAtlas == nullptr)
            return;

        Color penColor = currentPen.GetColor();
        penColor.a *= opacityStack.Last();

        if (!drawCmdList.IsEmpty() && drawCmdList.Last().fontSrg != fontAtlas->GetFontSrg2())
        {
            AddDrawCmd();

            drawCmdList.Last().fontSrg = fontAtlas->GetFontSrg2();
        }

        u32 color = penColor.ToU32();

	    for (int i = 0; i < length; ++i)
	    {
            char c = text[i];
            Rect quad = quads[i];

            if (ignoreCharacters.Exists(c))
            {
	            continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());
            if (glyph.charCode == 0)
            {
                continue;
            }

            Vec2 offset = Vec2();
            if (coordinateSpaceStack.Last().isTranslationOnly)
            {
                offset = coordinateSpaceStack.Last().translation;
            }

            quad = quad.Translate(textPos + offset);

            PrimReserve(4, 6);

            f32 atlasSize = glyph.atlasSize;

            Vec2 uvMin = Vec2((f32)glyph.x0 / atlasSize, (f32)glyph.y0 / atlasSize);
            Vec2 uvMax = Vec2((f32)glyph.x1 / atlasSize, (f32)glyph.y1 / atlasSize);

            Vec2 topLeft = quad.min;
            Vec2 topRight = Vec2(quad.max.x, quad.min.y);
            Vec2 bottomRight = Vec2(quad.max.x, quad.max.y);
            Vec2 bottomLeft = Vec2(quad.min.x, quad.max.y);

            Vec2 topLeftUV = uvMin;
            Vec2 topRightUV = Vec2(uvMax.x, uvMin.y);
            Vec2 bottomRightUV = uvMax;
            Vec2 bottomLeftUV = Vec2(uvMin.x, uvMax.y);

            FDrawIndex idx = vertexCurrentIdx;
            indexWritePtr[0] = idx; indexWritePtr[1] = (idx + 1); indexWritePtr[2] = (idx + 2);
            indexWritePtr[3] = idx; indexWritePtr[4] = (idx + 2); indexWritePtr[5] = (idx + 3);

            vertexWritePtr[0].position = topLeft; vertexWritePtr[0].color = color; vertexWritePtr[0].uv = topLeftUV; vertexWritePtr[0].drawType = DRAW_Text;
            vertexWritePtr[1].position = topRight; vertexWritePtr[1].color = color; vertexWritePtr[1].uv = topRightUV; vertexWritePtr[1].drawType = DRAW_Text;
            vertexWritePtr[2].position = bottomRight; vertexWritePtr[2].color = color; vertexWritePtr[2].uv = bottomRightUV; vertexWritePtr[2].drawType = DRAW_Text;
            vertexWritePtr[3].position = bottomLeft; vertexWritePtr[3].color = color; vertexWritePtr[3].uv = bottomLeftUV; vertexWritePtr[3].drawType = DRAW_Text;
            vertexWritePtr += 4;
            vertexCurrentIdx += 4;
            indexWritePtr += 6;

            drawCmdList.Last().numIndices += 6;
	    }
    }

    void FusionRenderer2::DrawSDFTextInternal(const Rect* quads, char* text, int length, const FFont& font,
	    Vec2 textPos)
    {
        thread_local HashSet<char> ignoreCharacters = { ' ', '\n', '\r', '\t', '\0' };

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = font.GetFamily();
        f32 fontSize = font.GetFontSize();

        if (fontSize <= 0)
            fontSize = fontManager->GetDefaultFontSize();
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        fontSize = Math::Max<f32>(fontSize, MinFontSize);

        Ref<FSDFFontAtlas> fontAtlas = fontManager->FindSDFFont(fontFamily);
        if (fontAtlas == nullptr)
            return;

        Color penColor = currentPen.GetColor();
        penColor.a *= opacityStack.Last();

        u32 color = penColor.ToU32();

        const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
        const f32 fontDpiScaling = dpi / 72.0f;
        const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
        const f32 metricsScaling = fontDpiScaling / systemDpiScaling;

        if (!drawCmdList.IsEmpty() && drawCmdList.Last().fontSrg != fontAtlas->GetFontSrg2())
        {
            AddDrawCmd();

            drawCmdList.Last().fontSrg = fontAtlas->GetFontSrg2();
        }

        for (int i = 0; i < length; ++i)
        {
            char c = text[i];
            Rect quad = quads[i];

            if (ignoreCharacters.Exists(c))
            {
                continue;
            }

            FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());
            if (glyph.charCode == 0)
            {
                continue;
            }

            Vec2 offset = Vec2();
            if (coordinateSpaceStack.Last().isTranslationOnly)
            {
                offset = coordinateSpaceStack.Last().translation;
            }

            quad = quad.Translate(textPos + offset);

            PrimReserve(4, 6);

            f32 atlasSize = glyph.atlasSize;

            f32 uvPadding = 2.5f / fontAtlas->GetAtlasSize();

            Vec2 uvMin = Vec2((f32)glyph.x0 / atlasSize, (f32)glyph.y0 / atlasSize) - Vec2(uvPadding, uvPadding);
            Vec2 uvMax = Vec2((f32)glyph.x1 / atlasSize, (f32)glyph.y1 / atlasSize) + Vec2(uvPadding, uvPadding);

            Vec2 topLeft = quad.min;
            Vec2 topRight = Vec2(quad.max.x, quad.min.y);
            Vec2 bottomRight = Vec2(quad.max.x, quad.max.y);
            Vec2 bottomLeft = Vec2(quad.min.x, quad.max.y);

            Vec2 topLeftUV = uvMin;
            Vec2 topRightUV = Vec2(uvMax.x, uvMin.y);
            Vec2 bottomRightUV = uvMax;
            Vec2 bottomLeftUV = Vec2(uvMin.x, uvMax.y);

            FDrawData drawData;
            drawData.uvMin = uvMin;
            drawData.uvMax = uvMax;
            drawData.rectSize = quad.GetSize() * fontDpiScaling * FusionApplication::Get()->GetDefaultScalingFactor() * 2.5f;

            drawDataArray.Insert(drawData);

            FDrawIndex idx = vertexCurrentIdx;
            indexWritePtr[0] = idx; indexWritePtr[1] = (idx + 1); indexWritePtr[2] = (idx + 2);
            indexWritePtr[3] = idx; indexWritePtr[4] = (idx + 2); indexWritePtr[5] = (idx + 3);

            for (int i = 0; i <= 3; ++i)
            {
                vertexWritePtr[i].index = drawDataArray.GetCount() - 1;
                vertexWritePtr[i].drawType = DRAW_SDFText;
            }

            vertexWritePtr[0].position = topLeft; vertexWritePtr[0].color = color; vertexWritePtr[0].uv = topLeftUV;
            vertexWritePtr[1].position = topRight; vertexWritePtr[1].color = color; vertexWritePtr[1].uv = topRightUV;
            vertexWritePtr[2].position = bottomRight; vertexWritePtr[2].color = color; vertexWritePtr[2].uv = bottomRightUV;
            vertexWritePtr[3].position = bottomLeft; vertexWritePtr[3].color = color; vertexWritePtr[3].uv = bottomLeftUV;
            vertexWritePtr += 4;
            vertexCurrentIdx += 4;
            indexWritePtr += 6;

            drawCmdList.Last().numIndices += 6;
            drawCmdList.Last().fontSrg = fontAtlas->GetFontSrg2();
        }
    }

    int FusionRenderer2::CalculateNumCircleSegments(float radius) const
    {
        const int radiusIndex = (int)(radius + 0.999999f); // ceil to never reduce accuracy
        return Math::Clamp((((((int)ceilf(Math::PI / acosf(1 - Math::Min((circleSegmentMaxError), (radius)) / (radius)))) + 1) / 2) * 2), 4, 512);
    }

    void FusionRenderer2::PathArcToFastInternal(const Vec2& center, float radius, int sampleMin, int sampleMax, int step)
    {
        if (radius < 0.5f)
        {
            PathInsert(center);
            return;
        }

        // Calculate arc auto segment step size
        if (step <= 0)
            step = ArcFastTableSize / CalculateNumCircleSegments(radius);

        // Make sure we never do steps larger than one quarter of the circle
        step = Math::Clamp(step, 1, (int)ArcFastTableSize / 4);

        const int sampleRange = abs(sampleMax - sampleMin);
        const int nextStep = step;

        int samples = sampleRange + 1;
        bool extraMaxSample = false;
        if (step > 1)
        {
            samples = sampleRange / step + 1;
            const int overstep = sampleRange % step;

            if (overstep > 0)
            {
                extraMaxSample = true;
                samples++;

                // When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
                // distribute first step range evenly between them by reducing first step size.
                if (sampleRange > 0)
                    step -= (step - overstep) / 2;
            }
        }

        path.InsertRange(samples, Vec2());

        Vec2* outPtr = path.GetData() + (path.GetCount() - samples);

        int sampleIndex = sampleMin;
        if (sampleIndex < 0 || sampleIndex >= ArcFastTableSize)
        {
            sampleIndex = sampleIndex % ArcFastTableSize;
            if (sampleIndex < 0)
                sampleIndex += ArcFastTableSize;
        }

        if (sampleMax >= sampleMin)
        {
            for (int a = sampleMin; a <= sampleMax; a += step, sampleIndex += step, step = nextStep)
            {
                if (sampleIndex >= ArcFastTableSize)
                    sampleIndex -= ArcFastTableSize;

                const Vec2 s = arcFastVertex[sampleIndex];
                outPtr->x = center.x + s.x * radius;
                outPtr->y = center.y + s.y * radius;
                PathMinMax(*outPtr);
                outPtr++;
            }
        }
        else
        {
            for (int a = sampleMin; a >= sampleMax; a -= step, sampleIndex -= step, step = nextStep)
            {
                if (sampleIndex < 0)
                    sampleIndex += ArcFastTableSize;

                const Vec2 s = arcFastVertex[sampleIndex];
                outPtr->x = center.x + s.x * radius;
                outPtr->y = center.y + s.y * radius;
                PathMinMax(*outPtr);
                outPtr++;
            }
        }

        if (extraMaxSample)
        {
            int normalizedMaxSample = sampleMax % ArcFastTableSize;
            if (normalizedMaxSample < 0)
                normalizedMaxSample += ArcFastTableSize;

            const Vec2 s = arcFastVertex[normalizedMaxSample];
            outPtr->x = center.x + s.x * radius;
            outPtr->y = center.y + s.y * radius;
            PathMinMax(*outPtr);
            outPtr++;
        }
    }

    void FusionRenderer2::AddDrawCmd()
    {
        ZoneScoped;

        if (coordinateSpaceStack.IsEmpty())
            return;

        // TODO: Implement vertex indexing with more than 65,535 vertices

        Ref<FFontManager> fontManager = FusionApplication::Get()->GetFontManager();

        Name fontFamily = currentFont.GetFamily();
        int fontSize = currentFont.GetFontSize();

        if (fontSize <= 0)
            fontSize = 12;
        if (!fontFamily.IsValid())
            fontFamily = fontManager->GetDefaultFontFamily();

        FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);

        FDrawCmd drawCmd{};
        drawCmd.rootConstants.transparentUV = transparentPixelUV;
        
        if (fontAtlas)
        {
            drawCmd.fontSrg = fontAtlas->GetFontSrg2();
        }

        if (drawCmdList.GetCount() > 0)
        {
            if (drawCmdList.Last().numIndices == 0) // Reuse previous draw command
            {
                if (fontAtlas)
                {
                    drawCmdList.Last().fontSrg = fontAtlas->GetFontSrg2();
                    drawCmdList.Last().firstInstance = coordinateSpaceStack.Last().indexInObjectArray;
                    drawCmdList.Last().textureSrgOverride = nullptr;
                    
                    drawCmdList.Last().rootConstants.numClipRects = clipStack.GetCount();
                    int j = 0;
                    for (int i = drawCmdList.Last().rootConstants.numClipRects - 1; i >= 0; --i)
                    {
                        drawCmdList.Last().rootConstants.clipRectIndices[j++] = clipStack[i];
                    }
                }
            }
            else
            {
                drawCmd.firstInstance = coordinateSpaceStack.Last().indexInObjectArray;
                drawCmd.vertexOffset = drawCmdList.Last().vertexOffset;
                drawCmd.indexOffset = (u32)indexArray.GetCount();
                drawCmd.numIndices = 0;
                drawCmd.textureSrgOverride = nullptr;
                
                drawCmd.rootConstants.numClipRects = clipStack.GetCount();
                int j = 0;
                for (int i = drawCmd.rootConstants.numClipRects - 1; i>= 0 ; --i)
                {
                    drawCmd.rootConstants.clipRectIndices[j++] = clipStack[i];
                }

                drawCmdList.Insert(drawCmd);
            }
        }
        else
        {
            drawCmdList.Insert(drawCmd);
        }
    }

    void FusionRenderer2::PrimReserve(int vertexCount, int indexCount)
    {
        ZoneScoped;

        // TODO: Add support for single item more than 65,536 vertices

        u32 curVertexCount = vertexArray.GetCount();

        if ((u32)vertexCurrentIdx + (u32)vertexCount >= (u32)NumericLimits<u16>::Max())
        {
            AddDrawCmd();

            drawCmdList.Last().vertexOffset = vertexCurrentIdx;
            vertexCurrentIdx = 0;
        }

        vertexArray.InsertRange(vertexCount);
        vertexWritePtr = vertexArray.GetData() + curVertexCount;

        int curIndexCount = indexArray.GetCount();
        indexArray.InsertRange(indexCount);
        indexWritePtr = indexArray.GetData() + curIndexCount;
    }

    void FusionRenderer2::PrimUnreserve(int vertexCount, int indexCount)
    {
        ZoneScoped;

        while (vertexCount > 0)
        {
            vertexArray.RemoveLast();
            vertexCount--;
        }

        while (indexCount > 0)
        {
            indexArray.RemoveLast();
            indexCount--;
        }
    }

    void FusionRenderer2::PrimRect(const Rect& quad, u32 color, Vec2* uvs, FDrawType drawType, int index)
    {
        Vec2 topLeft = quad.min;
        Vec2 topRight = Vec2(quad.max.x, quad.min.y);
        Vec2 bottomRight = Vec2(quad.max.x, quad.max.y);
        Vec2 bottomLeft = Vec2(quad.min.x, quad.max.y);

        Vec2 topLeftUV = uvs != nullptr ? uvs[0] : Vec2(0, 0);
        Vec2 topRightUV = uvs != nullptr ? uvs[1] : Vec2(1, 0);
        Vec2 bottomRightUV = uvs != nullptr ? uvs[2] : Vec2(1, 1);
        Vec2 bottomLeftUV = uvs != nullptr ? uvs[3] : Vec2(0, 1);

        FDrawIndex idx = vertexCurrentIdx;
        indexWritePtr[0] = idx; indexWritePtr[1] = (idx + 1); indexWritePtr[2] = (idx + 2);
        indexWritePtr[3] = idx; indexWritePtr[4] = (idx + 2); indexWritePtr[5] = (idx + 3);

        vertexWritePtr[0].position = topLeft; vertexWritePtr[0].color = color; vertexWritePtr[0].uv = topLeftUV; vertexWritePtr[0].drawType = drawType;
        vertexWritePtr[0].index = index;

    	vertexWritePtr[1].position = topRight; vertexWritePtr[1].color = color; vertexWritePtr[1].uv = topRightUV; vertexWritePtr[1].drawType = drawType;
        vertexWritePtr[1].index = index;

    	vertexWritePtr[2].position = bottomRight; vertexWritePtr[2].color = color; vertexWritePtr[2].uv = bottomRightUV; vertexWritePtr[2].drawType = drawType;
        vertexWritePtr[2].index = index;

        vertexWritePtr[3].position = bottomLeft; vertexWritePtr[3].color = color; vertexWritePtr[3].uv = bottomLeftUV; vertexWritePtr[3].drawType = drawType;
        vertexWritePtr[3].index = index;

        vertexWritePtr += 4;
        vertexCurrentIdx += 4;
        indexWritePtr += 6;

        drawCmdList.Last().numIndices += 6;
    }

    void FusionRenderer2::PathInsert(const Vec2& point)
    {
        ZoneScoped;

        path.Insert(point);

        PathMinMax(point);
    }

    void FusionRenderer2::PathMinMax(const Vec2& point)
    {
        ZoneScoped;

        pathMin.x = Math::Min(point.x, pathMin.x);
        pathMin.y = Math::Min(point.y, pathMin.y);

        pathMax.x = Math::Max(point.x, pathMax.x);
        pathMax.y = Math::Max(point.y, pathMax.y);
    }

    bool FusionRenderer2::AddRect(const Rect& rect, const Vec4& cornerRadius, bool antiAliased)
    {
        ZoneScoped;

        u32 color = currentPen.GetColor().ToU32();

        if ((color & ColorAlphaMask) == 0)
            return true;

        PathClear();
        if (antiAliased)
            PathRect(Rect(rect.min + Vec2(0.50f, 0.50f), rect.max - Vec2(0.50f, 0.50f)), cornerRadius);
        else
            PathRect(Rect(rect.min + Vec2(0.50f, 0.50f), rect.max - Vec2(0.49f, 0.49f)), cornerRadius); // Better looking lower-right corner and rounded non-AA shapes.

        return PathStroke(true, antiAliased);
    }

    bool FusionRenderer2::AddRectFilled(Rect rect, const Vec4& cornerRadius, bool antiAliased)
    {
        ZoneScoped;

        Color fillColor = currentBrush.GetFillColor();
        fillColor.a *= opacityStack.Last();
        u32 color = fillColor.ToU32();

        if ((color & ColorAlphaMask) == 0)
            return true;

        PathClear();
        PathRect(rect, cornerRadius);
        return PathFill(antiAliased);
    }

    void FusionRenderer2::AddConvexPolyFilled(const Vec2* points, int numPoints, bool antiAliased, Rect* minMaxPos)
    {
        ZoneScoped;

        if (points == nullptr || numPoints <= 0)
            return;

        if (opacityStack.Last() < MinOpacity)
            return;

        u32 color;
        FDrawType drawType = DRAW_Geometry;
        bool usesTextureSrgOverride = false;

        if (currentBrush.GetBrushStyle() == FBrushStyle::Image && minMaxPos != nullptr)
        {
            Color tint = currentBrush.GetFillColor();
            tint.a *= opacityStack.Last();

            color = tint.ToU32();
            drawType = DRAW_TextureNoTile;

            Vec2 brushSize = currentBrush.GetBrushSize();
            Vec2 brushPos = currentBrush.GetBrushPosition();
            FImageFit imageFit = currentBrush.GetImageFit();
            FBrushTiling tiling = currentBrush.GetBrushTiling();

            bool tiledY = tiling == FBrushTiling::TileXY || tiling == FBrushTiling::TileY;
            bool tiledX = tiling == FBrushTiling::TileXY || tiling == FBrushTiling::TileX;
            bool autoSizeX = brushSize.x < 0;
            bool autoSizeY = brushSize.y < 0;

            FDrawData drawData{};
            auto app = FusionApplication::Get();

            String imageName = currentBrush.GetImageName().GetString();

            FImageAtlas::ImageItem image = app->GetImageAtlas()->FindImage(currentBrush.GetImageName());
            if (!image.IsValid())
            {
	            CMImage imageAsset = app->LoadImageAsset(currentBrush.GetImageName());
                if (imageAsset.IsValid())
                {
                	image = app->GetImageAtlas()->AddImage(currentBrush.GetImageName(), imageAsset);
                }
            }

            if (!image.IsValid() && imageName.StartsWith(FFontAtlas::ImageNamePrefix))
            {
                drawType = DRAW_FontAtlas;
                // Example: __FontAtlas_Roboto_0
                imageName.Remove(0, strlen(FFontAtlas::ImageNamePrefix));
                Array<String> split = imageName.Split('_');
                if (split.GetSize() >= 2)
                {
                    String fontName = split[0];
                    int layerIndex = -1;
                    if (String::TryParse(split[1], layerIndex) && layerIndex >= 0)
                    {
                        FFontAtlas* fontAtlas = FusionApplication::Get()->GetFontManager()->FindFont(fontName);
                        if (fontAtlas)
                        {
                            image = { .layerIndex = layerIndex, .uvMin = Vec2(0, 0), .uvMax = Vec2(1, 1), .width = fontAtlas->GetAtlasSize(), .height = fontAtlas->GetAtlasSize() };
                        }
                    }
                }
            }

            if (!image.IsValid() && currentBrush.GetImageName().GetString().StartsWith("/Temp"))
            {
                auto [textureView, textureSrgOverride] = app->LoadRawTextureAtPath(currentBrush.GetImageName());
                if (textureView && textureSrgOverride && textureView->GetTexture())
                {
					RHI::Texture* rhiTexture = textureView->GetTexture();

                    AddDrawCmd();

                    drawCmdList.Last().textureSrgOverride = textureSrgOverride;

                    image.layerIndex = 0;
					image.width = rhiTexture->GetWidth();
					image.height = rhiTexture->GetHeight();
					image.uvMin = Vec2(0, 0);
					image.uvMax = Vec2(1, 1);

                    usesTextureSrgOverride = true;
                }
            }

            if (image.IsValid())
            {
                Vec2 imageSize = Vec2(image.width, image.height);
                drawData.rectSize = minMaxPos->GetSize();
                drawData.index = image.layerIndex;
                drawData.imageFit = (int)imageFit;
                drawData.uvMin = image.uvMin;
                drawData.uvMax = image.uvMax;

                switch (imageFit)
                {
                case FImageFit::Default:
                case FImageFit::Fill:
                    if (autoSizeX)
                        brushSize.x = drawData.rectSize.width;
                    if (autoSizeY)
                        brushSize.y = drawData.rectSize.height;
	                break;
                case FImageFit::Contain:
                    if (autoSizeX && autoSizeY)
                    {
                        float scale = Math::Min(drawData.rectSize.width / imageSize.width, drawData.rectSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
                    else if (autoSizeX)
                    {
                        float scale = Math::Min(drawData.rectSize.width / imageSize.width, brushSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
                    else if (autoSizeY)
                    {
                        float scale = Math::Min(brushSize.width / imageSize.width, drawData.rectSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
                    else
                    {
                        float scale = Math::Min(brushSize.width / imageSize.width, brushSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
	                break;
                case FImageFit::Cover:
                    if (autoSizeX && autoSizeY)
                    {
                        float scale = Math::Max(drawData.rectSize.width / imageSize.width, drawData.rectSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
                    else if (autoSizeX)
                    {
                        float scale = Math::Max(drawData.rectSize.width / imageSize.width, brushSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
                    else if (autoSizeY)
                    {
                        float scale = Math::Max(brushSize.width / imageSize.width, drawData.rectSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
                    else
                    {
                        float scale = Math::Max(brushSize.width / imageSize.width, brushSize.height / imageSize.height);
                        brushSize.x = imageSize.width * scale;
                        brushSize.y = imageSize.height * scale;
                    }
	                break;
                }

                if (drawType != DRAW_FontAtlas)
                {
                    switch (currentBrush.GetBrushTiling())
                    {
                    case FBrushTiling::None:
                        drawType = DRAW_TextureNoTile;
                        break;
                    case FBrushTiling::TileX:
                        drawType = DRAW_TextureTileX;
                        break;
                    case FBrushTiling::TileY:
                        drawType = DRAW_TextureTileY;
                        break;
                    case FBrushTiling::TileXY:
                        drawType = DRAW_TextureTileXY;
                        break;
                    }
                }

                drawData.brushSize = brushSize;
                drawData.brushPos = brushPos;

                drawDataArray.Insert(drawData);
            }
        }
        else if (currentBrush.GetBrushStyle() == FBrushStyle::Gradient && minMaxPos != nullptr && 
            currentBrush.GetGradient().stops.GetSize() >= 2)
        {
            const FGradient& gradient = currentBrush.GetGradient();

            Color tint = currentBrush.GetFillColor();
            tint.a *= opacityStack.Last();

            color = tint.ToU32();
            drawType = gradient.gradientType == FGradientType::Linear ? DRAW_LinearGradient : DRAW_RadialGradient;

            Vec2 brushSize = currentBrush.GetBrushSize();
            Vec2 brushPos = currentBrush.GetBrushPosition();
            FImageFit imageFit = currentBrush.GetImageFit();
            FBrushTiling tiling = currentBrush.GetBrushTiling();

            bool tiledY = tiling == FBrushTiling::TileXY || tiling == FBrushTiling::TileY;
            bool tiledX = tiling == FBrushTiling::TileXY || tiling == FBrushTiling::TileX;
            bool autoSizeX = brushSize.x < 0;
            bool autoSizeY = brushSize.y < 0;

            FDrawData drawData{};
        	drawData.rectSize = minMaxPos->GetSize();
            drawData.userAngle = gradient.angle * DEG_TO_RAD;

            if (autoSizeX)
                brushSize.x = drawData.rectSize.x;
            if (autoSizeY)
                brushSize.y = drawData.rectSize.y;

            switch (imageFit)
            {
            case FImageFit::Default:
                break;
            case FImageFit::Fill:
	            break;
            case FImageFit::Contain:
                if (autoSizeX && autoSizeY)
                {
                    float scale = Math::Min(drawData.rectSize.width / brushSize.width, drawData.rectSize.height / brushSize.height);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
                else if (autoSizeX)
                {
                    float scale = Math::Min(drawData.rectSize.width / brushSize.width, brushSize.height / brushSize.height);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
                else if (autoSizeY)
                {
                    float scale = Math::Min(brushSize.width / brushSize.width, drawData.rectSize.height / brushSize.height);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
                else
                {
                    float scale = Math::Min(brushSize.width / brushSize.width, brushSize.height / brushSize.height);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
	            break;
            case FImageFit::Cover:
                if (autoSizeX && autoSizeY)
                {
                    float scale = Math::Max(drawData.rectSize.width / brushSize.width, drawData.rectSize.height / brushSize.height);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
                else if (autoSizeX)
                {
                    float scale = Math::Max(drawData.rectSize.width / brushSize.width, 1.0f);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
                else if (autoSizeY)
                {
                    float scale = Math::Max(1.0f, drawData.rectSize.height / brushSize.height);
                    brushSize.x = brushSize.width * scale;
                    brushSize.y = brushSize.height * scale;
                }
	            break;
            }

            drawData.brushPos = brushPos;
            drawData.brushSize = brushSize;

            drawData.index = (int)gradientKeyArray.GetCount();
            drawData.endIndex = drawData.index + (int)gradient.stops.GetSize() - 1;

            for (const auto& stop : gradient.stops)
            {
	            gradientKeyArray.Insert(FColorStop{ .color = stop.color.ToVec4(), .position = stop.position });
            }

            drawDataArray.Insert(drawData);
        }
        else
        {
            minMaxPos = nullptr;
            Color fill = currentBrush.GetFillColor();
            fill.a *= opacityStack.Last();

            color = fill.ToU32();
        }

        auto calculateUV = [this, minMaxPos](Vec2 pos) -> Vec2
            {
                if (minMaxPos != nullptr)
                {
                    return Vec2(((pos.x - minMaxPos->min.x) / (minMaxPos->max.x - minMaxPos->min.x)),
                        ((pos.y - minMaxPos->min.y) / (minMaxPos->max.y - minMaxPos->min.y)));
                }
                return whitePixelUV;
            };

        Vec2 offset = Vec2();
        if (coordinateSpaceStack.Last().isTranslationOnly)
        {
            offset = coordinateSpaceStack.Last().translation;
        }
        
        if (antiAliased)
        {
            // Credit: Dear ImGui

            const float AA_SIZE = fringeScale;
            const u32 transparentColor = color & ~ColorAlphaMask;
            const int indexCount = (numPoints - 2) * 3 + numPoints * 6;
            const int vertexCount = (numPoints * 2);
            PrimReserve(vertexCount, indexCount);
            
            // Add indexes for fill
            unsigned int vertexInnerIdx = vertexCurrentIdx;
            unsigned int vertexOuterIdx = vertexCurrentIdx + 1;
            for (int i = 2; i < numPoints; i++)
            {
                indexWritePtr[0] = (FDrawIndex)(vertexInnerIdx); indexWritePtr[1] = (FDrawIndex)(vertexInnerIdx + ((i - 1) << 1)); indexWritePtr[2] = (FDrawIndex)(vertexInnerIdx + (i << 1));
                indexWritePtr += 3;
            }

            // Compute normals
            temporaryPoints.RemoveAll();
            temporaryPoints.InsertRange(numPoints);
            Vec2* tempNormals = temporaryPoints.GetData();
            
            for (int i0 = numPoints - 1, i1 = 0; i1 < numPoints; i0 = i1++)
            {
                const Vec2& p0 = points[i0] + offset;
                const Vec2& p1 = points[i1] + offset;
                float dx = p1.x - p0.x;
                float dy = p1.y - p0.y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                tempNormals[i0].x = dy;
                tempNormals[i0].y = -dx;
            }

            for (int i0 = numPoints - 1, i1 = 0; i1 < numPoints; i0 = i1++)
            {
                // Average normals
                const Vec2& n0 = tempNormals[i0];
                const Vec2& n1 = tempNormals[i1];
                float dm_x = (n0.x + n1.x) * 0.5f;
                float dm_y = (n0.y + n1.y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                dm_x *= AA_SIZE * 0.5f;
                dm_y *= AA_SIZE * 0.5f;

                //Vec2 pos0 = Vec2(points[i1].x - dm_x, points[i1].y - dm_y);
                //Vec2 pos1 = Vec2(points[i1].x + dm_x, points[i1].y + dm_y);

                Vec2 pos0 = Vec2(points[i1].x, points[i1].y);
                Vec2 pos1 = Vec2(points[i1].x + dm_x * 2, points[i1].y + dm_y * 2);
                
                // Add vertices

                // Inner
                vertexWritePtr[0].position.x = pos0.x + offset.x; vertexWritePtr[0].position.y = pos0.y + offset.y; vertexWritePtr[0].uv = calculateUV(pos0); vertexWritePtr[0].color = color;        // Inner
                // Outer
            	vertexWritePtr[1].position.x = pos1.x + offset.x; vertexWritePtr[1].position.y = pos1.y + offset.y; vertexWritePtr[1].uv = calculateUV(pos1); vertexWritePtr[1].color = transparentColor;  // Outer
                vertexWritePtr[0].drawType = drawType; vertexWritePtr[0].index = (int)drawDataArray.GetCount() - 1;
                vertexWritePtr[1].drawType = drawType; vertexWritePtr[1].index = (int)drawDataArray.GetCount() - 1;
                
            	vertexWritePtr += 2;
                
                // Add indexes for fringes
                indexWritePtr[0] = (FDrawIndex)(vertexInnerIdx + (i1 << 1)); indexWritePtr[1] = (FDrawIndex)(vertexInnerIdx + (i0 << 1)); indexWritePtr[2] = (FDrawIndex)(vertexOuterIdx + (i0 << 1));
                indexWritePtr[3] = (FDrawIndex)(vertexOuterIdx + (i0 << 1)); indexWritePtr[4] = (FDrawIndex)(vertexOuterIdx + (i1 << 1)); indexWritePtr[5] = (FDrawIndex)(vertexInnerIdx + (i1 << 1));
                indexWritePtr += 6;
            }

            vertexCurrentIdx += vertexCount;
            drawCmdList.Last().numIndices += indexCount;
        }
        else
        {
            // Credit: Dear-ImGui

            const int indexCount = (numPoints - 2) * 3;
            const int vertexCount = numPoints;
            PrimReserve(vertexCount, indexCount);

            for (int i = 0; i < vertexCount; i++)
            {
                vertexWritePtr[0].position = points[i] + offset;
                vertexWritePtr[0].uv = calculateUV(points[i]);
                vertexWritePtr[0].color = color;
                vertexWritePtr[0].drawType = drawType;
                vertexWritePtr[0].index = (int)drawDataArray.GetCount() - 1;
                vertexWritePtr++;
            }

            for (int i = 2; i < numPoints; ++i)
            {
                indexWritePtr[0] = vertexCurrentIdx;
                indexWritePtr[1] = vertexCurrentIdx + (i - 1);
                indexWritePtr[2] = vertexCurrentIdx + (i);
                indexWritePtr += 3;
            }

            vertexCurrentIdx += vertexCount;
            drawCmdList.Last().numIndices += indexCount;
        }

        if (usesTextureSrgOverride)
        {
            AddDrawCmd();
        }
    }

    void FusionRenderer2::AddPolyLine(const Vec2* points, int numPoints, f32 thickness, bool closed, bool antiAliased)
    {
        ZoneScoped;

        if (points == nullptr || numPoints <= 0)
            return;

        Color penColor = currentPen.GetColor();
        penColor.a *= opacityStack.Last();

        u32 color = penColor.ToU32();

        const Vec2 uv = whitePixelUV;
        const int count = closed ? numPoints : numPoints - 1; // The number of line segments we need to draw
        const bool thickLine = (thickness > fringeScale);

        Vec2 offset = Vec2();
        if (coordinateSpaceStack.Last().isTranslationOnly)
        {
            offset = coordinateSpaceStack.Last().translation;
        }

        // Credit: Dear ImGui

        if (antiAliased)
        {
	        // [PATH 1]
            // Anti-aliased stroke
            const float AA_SIZE = fringeScale;
            const u32 transparentColor = color & ~ColorAlphaMask;

            // Thicknesses <1.0 should behave like thickness 1.0
            thickness = Math::Max(thickness, 1.0f);
            const int integerThickness = (int)thickness;
            const float fractionalThickness = thickness - integerThickness;

            const int indexCount = (thickLine ? count * 18 : count * 12);
            const int vertexCount = (thickLine ? numPoints * 4 : numPoints * 3);
            PrimReserve(vertexCount, indexCount);

            // Temporary buffer
			// The first <numPoints> items are normals at each line point, then after that there are either 2 or 4 temp points for each line point
            temporaryPoints.RemoveAll();
            temporaryPoints.InsertRange(numPoints * (thickLine ? 5 : 3));
            Vec2* tempNormals = temporaryPoints.GetData();
            Vec2* tempPoints = tempNormals + numPoints;

            // Calculate normals (tangents) for each line segment
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                float dx = points[i2].x - points[i1].x;
                float dy = points[i2].y - points[i1].y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                tempNormals[i1].x = dy;
                tempNormals[i1].y = -dx;
            }

            if (!closed)
                tempNormals[numPoints - 1] = tempNormals[numPoints - 2];

            if (!thickLine)
            {
                const float half_draw_size = AA_SIZE;

                // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
                if (!closed)
                {
                    tempPoints[0] = points[0] + tempNormals[0] * half_draw_size + offset;
                    tempPoints[1] = points[0] - tempNormals[0] * half_draw_size + offset;
                    tempPoints[(numPoints - 1) * 2 + 0] = points[numPoints - 1] + tempNormals[numPoints - 1] * half_draw_size + offset;
                    tempPoints[(numPoints - 1) * 2 + 1] = points[numPoints - 1] - tempNormals[numPoints - 1] * half_draw_size + offset;
                }

                // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
                // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                unsigned int idx1 = vertexCurrentIdx; // Vertex index for start of line segment
                for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
                {
                    const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1; // i2 is the second point of the line segment
                    const unsigned int idx2 = ((i1 + 1) == numPoints) ? vertexCurrentIdx : (idx1 + 3); // Vertex index for end of segment

                    // Average normals
                    float dm_x = (tempNormals[i1].x + tempNormals[i2].x) * 0.5f;
                    float dm_y = (tempNormals[i1].y + tempNormals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    dm_x *= half_draw_size; // dm_x, dm_y are offset to the outer edge of the AA area
                    dm_y *= half_draw_size;

                    // Add temporary vertexes for the outer edges
                    Vec2* out_vtx = &tempPoints[i2 * 2];
                    out_vtx[0].x = points[i2].x + dm_x + offset.x;
                    out_vtx[0].y = points[i2].y + dm_y + offset.y;
                    out_vtx[1].x = points[i2].x - dm_x + offset.x;
                    out_vtx[1].y = points[i2].y - dm_y + offset.y;

                    {
                        // Add indexes for four triangles
                        indexWritePtr[0] = (FDrawIndex)(idx2 + 0); indexWritePtr[1] = (FDrawIndex)(idx1 + 0); indexWritePtr[2] = (FDrawIndex)(idx1 + 2); // Right tri 1
                        indexWritePtr[3] = (FDrawIndex)(idx1 + 2); indexWritePtr[4] = (FDrawIndex)(idx2 + 2); indexWritePtr[5] = (FDrawIndex)(idx2 + 0); // Right tri 2
                        indexWritePtr[6] = (FDrawIndex)(idx2 + 1); indexWritePtr[7] = (FDrawIndex)(idx1 + 1); indexWritePtr[8] = (FDrawIndex)(idx1 + 0); // Left tri 1
                        indexWritePtr[9] = (FDrawIndex)(idx1 + 0); indexWritePtr[10] = (FDrawIndex)(idx2 + 0); indexWritePtr[11] = (FDrawIndex)(idx2 + 1); // Left tri 2
                        indexWritePtr += 12;
                    }

                    idx1 = idx2;
                }

                {
                    // If we're not using a texture, we need the center vertex as well
                    for (int i = 0; i < numPoints; i++)
                    {
                        vertexWritePtr[0].position = points[i] + offset;              vertexWritePtr[0].uv = whitePixelUV; vertexWritePtr[0].color = color;       // Center of line
                        vertexWritePtr[1].position = tempPoints[i * 2 + 0]; vertexWritePtr[1].uv = whitePixelUV; vertexWritePtr[1].color = transparentColor; // Left-side outer edge
                        vertexWritePtr[2].position = tempPoints[i * 2 + 1]; vertexWritePtr[2].uv = whitePixelUV; vertexWritePtr[2].color = transparentColor; // Right-side outer edge
                        vertexWritePtr += 3;
                    }
                }
            }
            else
            {
                // [PATH 2] Non texture-based lines (thick): we need to draw the solid line core and thus require four vertices per point
                const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;

                // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
                if (!closed)
                {
                    const int points_last = numPoints - 1;
                    tempPoints[0] = points[0] + offset + tempNormals[0] * (half_inner_thickness + AA_SIZE);
                    tempPoints[1] = points[0] + offset + tempNormals[0] * (half_inner_thickness);
                    tempPoints[2] = points[0] + offset - tempNormals[0] * (half_inner_thickness);
                    tempPoints[3] = points[0] + offset - tempNormals[0] * (half_inner_thickness + AA_SIZE);
                    tempPoints[points_last * 4 + 0] = points[points_last] + offset + tempNormals[points_last] * (half_inner_thickness + AA_SIZE);
                    tempPoints[points_last * 4 + 1] = points[points_last] + offset + tempNormals[points_last] * (half_inner_thickness);
                    tempPoints[points_last * 4 + 2] = points[points_last] + offset - tempNormals[points_last] * (half_inner_thickness);
                    tempPoints[points_last * 4 + 3] = points[points_last] + offset - tempNormals[points_last] * (half_inner_thickness + AA_SIZE);
                }

                // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
                // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                unsigned int idx1 = vertexCurrentIdx; // Vertex index for start of line segment
                for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
                {
                    const int i2 = (i1 + 1) == numPoints ? 0 : (i1 + 1); // i2 is the second point of the line segment
                    const unsigned int idx2 = (i1 + 1) == numPoints ? vertexCurrentIdx : (idx1 + 4); // Vertex index for end of segment

                    // Average normals
                    float dm_x = (tempNormals[i1].x + tempNormals[i2].x) * 0.5f;
                    float dm_y = (tempNormals[i1].y + tempNormals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                    float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                    float dm_in_x = dm_x * half_inner_thickness;
                    float dm_in_y = dm_y * half_inner_thickness;

                    // Add temporary vertices
                    Vec2* out_vtx = &tempPoints[i2 * 4];
                    out_vtx[0].x = points[i2].x + dm_out_x + offset.x;
                    out_vtx[0].y = points[i2].y + dm_out_y + offset.y;
                    out_vtx[1].x = points[i2].x + dm_in_x + offset.x;
                    out_vtx[1].y = points[i2].y + dm_in_y + offset.y;
                    out_vtx[2].x = points[i2].x - dm_in_x + offset.x;
                    out_vtx[2].y = points[i2].y - dm_in_y + offset.y;
                    out_vtx[3].x = points[i2].x - dm_out_x + offset.x;
                    out_vtx[3].y = points[i2].y - dm_out_y + offset.y;

                    // Add indexes
                    indexWritePtr[0] = (FDrawIndex)(idx2 + 1); indexWritePtr[1] = (FDrawIndex)(idx1 + 1); indexWritePtr[2] = (FDrawIndex)(idx1 + 2);
                    indexWritePtr[3] = (FDrawIndex)(idx1 + 2); indexWritePtr[4] = (FDrawIndex)(idx2 + 2); indexWritePtr[5] = (FDrawIndex)(idx2 + 1);
                    indexWritePtr[6] = (FDrawIndex)(idx2 + 1); indexWritePtr[7] = (FDrawIndex)(idx1 + 1); indexWritePtr[8] = (FDrawIndex)(idx1 + 0);
                    indexWritePtr[9] = (FDrawIndex)(idx1 + 0); indexWritePtr[10] = (FDrawIndex)(idx2 + 0); indexWritePtr[11] = (FDrawIndex)(idx2 + 1);
                    indexWritePtr[12] = (FDrawIndex)(idx2 + 2); indexWritePtr[13] = (FDrawIndex)(idx1 + 2); indexWritePtr[14] = (FDrawIndex)(idx1 + 3);
                    indexWritePtr[15] = (FDrawIndex)(idx1 + 3); indexWritePtr[16] = (FDrawIndex)(idx2 + 3); indexWritePtr[17] = (FDrawIndex)(idx2 + 2);
                    indexWritePtr += 18;

                    idx1 = idx2;
                }

                // Add vertices
                for (int i = 0; i < numPoints; i++)
                {
                    vertexWritePtr[0].position = tempPoints[i * 4 + 0]; vertexWritePtr[0].uv = whitePixelUV; vertexWritePtr[0].color = transparentColor;
                    vertexWritePtr[1].position = tempPoints[i * 4 + 1]; vertexWritePtr[1].uv = whitePixelUV; vertexWritePtr[1].color = color;
                    vertexWritePtr[2].position = tempPoints[i * 4 + 2]; vertexWritePtr[2].uv = whitePixelUV; vertexWritePtr[2].color = color;
                    vertexWritePtr[3].position = tempPoints[i * 4 + 3]; vertexWritePtr[3].uv = whitePixelUV; vertexWritePtr[3].color = transparentColor;
                    vertexWritePtr += 4;
                }
            }

            vertexCurrentIdx += vertexCount;
            drawCmdList.Last().numIndices += indexCount;
        }
        else
        {
            // [PATH 4] Non texture-based, Non anti-aliased lines
            const int indexCount = count * 6;
            const int vertexCount = count * 4;    // FIXME-OPT: Not sharing edges
            PrimReserve(vertexCount, indexCount);

            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                const Vec2& p1 = points[i1] + offset;
                const Vec2& p2 = points[i2] + offset;

                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                dx *= (thickness * 0.5f);
                dy *= (thickness * 0.5f);

                vertexWritePtr[0].position.x = p1.x + dy; vertexWritePtr[0].position.y = p1.y - dx; vertexWritePtr[0].uv = whitePixelUV; vertexWritePtr[0].color = color;
                vertexWritePtr[1].position.x = p2.x + dy; vertexWritePtr[1].position.y = p2.y - dx; vertexWritePtr[1].uv = whitePixelUV; vertexWritePtr[1].color = color;
                vertexWritePtr[2].position.x = p2.x - dy; vertexWritePtr[2].position.y = p2.y + dx; vertexWritePtr[2].uv = whitePixelUV; vertexWritePtr[2].color = color;
                vertexWritePtr[3].position.x = p1.x - dy; vertexWritePtr[3].position.y = p1.y + dx; vertexWritePtr[3].uv = whitePixelUV; vertexWritePtr[3].color = color;
                vertexWritePtr += 4;

                indexWritePtr[0] = (FDrawIndex)(vertexCurrentIdx); indexWritePtr[1] = (FDrawIndex)(vertexCurrentIdx + 1); indexWritePtr[2] = (FDrawIndex)(vertexCurrentIdx + 2);
                indexWritePtr[3] = (FDrawIndex)(vertexCurrentIdx); indexWritePtr[4] = (FDrawIndex)(vertexCurrentIdx + 2); indexWritePtr[5] = (FDrawIndex)(vertexCurrentIdx + 3);
                indexWritePtr += 6;
                vertexCurrentIdx += 4;
            }

            vertexCurrentIdx += vertexCount;
            drawCmdList.Last().numIndices += indexCount;
        }
    }

    bool FusionRenderer2::AddCircle(const Vec2& center, f32 radius, int numSegments, bool antiAliased)
    {
        ZoneScoped;

        u32 color = currentPen.GetColor().ToU32();
        if ((color & ColorAlphaMask) == 0 || radius < 0.5f)
            return true;

        if (numSegments <= 0)
        {
            PathArcToFast(center, radius - 0.5f, 0, 12);
            path.RemoveLast();
        }
        else
        {
            // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
            numSegments = Math::Clamp(numSegments, 3, CircleAutoSegmentMax);

            // Because we are filling a closed shape we remove 1 from the count of segments/points
            const float angleMax = (Math::PI * 2.0f) * ((float)numSegments - 1.0f) / (float)numSegments;
            PathArcTo(center, radius - 0.5f, 0, angleMax, numSegments - 1);
        }

        return PathStroke(true, antiAliased);
    }

    bool FusionRenderer2::AddCircleFilled(const Vec2& center, f32 radius, int numSegments, bool antiAliased)
    {
        ZoneScoped;

        u32 color = currentBrush.GetFillColor().ToU32();
        if ((color & ColorAlphaMask) == 0 || radius < 0.5f)
            return true;

        if (numSegments <= 0)
        {
            PathArcToFast(center, radius, 0, 12);
            path.RemoveLast();
        }
        else
        {
            // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
            numSegments = Math::Clamp(numSegments, 3, CircleAutoSegmentMax);

            // Because we are filling a closed shape we remove 1 from the count of segments/points
            const float angleMax = (Math::PI * 2.0f) * ((float)numSegments - 1.0f) / (float)numSegments;
            PathArcTo(center, radius, 0, angleMax, numSegments - 1);
        }

        return PathFill(antiAliased);
    }

    void FusionRenderer2::GrowQuadBuffer(u64 newTotalSize)
    {
        ZoneScoped;

	    meshBufferGrowRatio = Math::Min<float>(meshBufferGrowRatio, 0.75f);

        u64 curSize = quadsBuffer[0]->GetBufferSize();
        newTotalSize = Math::Max(curSize, newTotalSize);

        newTotalSize = Math::Max(newTotalSize, (u64)(curSize * (1 + meshBufferGrowRatio)));

	    for (int i = 0; i < numFrames; ++i)
	    {
            RHI::Buffer* original = quadsBuffer[i];

            RHI::BufferDescriptor quadMeshBufferDesc{};
            quadMeshBufferDesc.name = "Quad Meshes";
            quadMeshBufferDesc.bindFlags = RHI::BufferBindFlags::IndexBuffer | RHI::BufferBindFlags::VertexBuffer;
            quadMeshBufferDesc.bufferSize = newTotalSize;
            quadMeshBufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
            quadMeshBufferDesc.structureByteStride = quadMeshBufferDesc.bufferSize;

            quadsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(quadMeshBufferDesc);
            quadUpdatesRequired[i] = true;

            void* src;
            original->Map(0, original->GetBufferSize(), &src);
            {
                quadsBuffer[i]->UploadData(src, original->GetBufferSize());
            }
            original->Unmap();

            QueueDestroy(original);
	    }

    }

    void FusionRenderer2::QueueDestroy(RHI::Buffer* buffer)
    {
        destructionQueue.Add({ .buffer = buffer });
    }

} // namespace CE

