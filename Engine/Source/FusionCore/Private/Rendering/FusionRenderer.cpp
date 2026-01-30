#include "FusionCore.h"

namespace CE
{
	constexpr bool ForceDisableBatching = true;
	constexpr f32 StructuredBufferGrowRatio = 0.25f;

	FusionRenderer::FusionRenderer()
	{
		
	}

	void FusionRenderer::Init(const FusionRendererInitInfo& initInfo)
	{
		fusionShader = initInfo.fusionShader;
		multisampling = initInfo.multisampling;

		numFrames = RHI::FrameScheduler::Get()->GetFramesInFlight();

		drawItemsBuffer.Init("DrawItems_" + GetName().GetString(), initialDrawItemCapacity, numFrames);
		clipItemsBuffer.Init("ClipItems_" + GetName().GetString(), initialClipItemCapacity, numFrames);
		clipItemIndexListBuffer.Init("ClipItemIndices_" + GetName().GetString(), initialClipItemCapacity, numFrames);
		shapeItemsBuffer.Init("ShapeItems_" + GetName().GetString(), initialShapeItemCapacity, numFrames);
		lineItemsBuffer.Init("LineItems_" + GetName().GetString(), initialLineItemCapacity, numFrames);

		RHI::ShaderResourceGroupDescriptor perDrawSrgDesc{};
		perDrawSrgDesc.name = "Fusion SRG_PerDraw";
		perDrawSrgDesc.layout = FusionApplication::Get()->perDrawSrgLayout;

		RHI::ShaderResourceGroupDescriptor perViewSrgDesc{};
		perViewSrgDesc.name = "Fusion SRG_PerView";
		perViewSrgDesc.layout = FusionApplication::Get()->perViewSrgLayout;

		drawItemSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perDrawSrgDesc);
		perViewSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perViewSrgDesc);
		
		for (int i = 0; i < numFrames; ++i)
		{
			RHI::BufferDescriptor viewConstantsBufferDesc{};
			viewConstantsBufferDesc.name = "ViewConstants";
			viewConstantsBufferDesc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
			viewConstantsBufferDesc.bufferSize = sizeof(RPI::PerViewConstants);
			viewConstantsBufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
			viewConstantsBufferDesc.structureByteStride = sizeof(RPI::PerViewConstants);

			viewConstantsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(viewConstantsBufferDesc);
			viewConstantsUpdateRequired[i] = true;

			RHI::BufferDescriptor drawDataConstantBufferDesc{};
			drawDataConstantBufferDesc.name = "DrawDataConstants";
			drawDataConstantBufferDesc.bindFlags = BufferBindFlags::ConstantBuffer;
			drawDataConstantBufferDesc.bufferSize = sizeof(FDrawDataConstants);
			drawDataConstantBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			drawDataConstantBufferDesc.structureByteStride = sizeof(FDrawDataConstants);

			drawDataConstantsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(drawDataConstantBufferDesc);

			FDrawDataConstants data;
			data.frameIndex = i;

			drawDataConstantsBuffer[i]->UploadData(&data, drawDataConstantBufferDesc.bufferSize);

			perViewSrg->Bind(i, "_PerViewData", viewConstantsBuffer[i]);
			drawItemSrg->Bind(i, "_DrawList", drawItemsBuffer.GetBuffer(i));
			drawItemSrg->Bind(i, "_ClipItems", clipItemsBuffer.GetBuffer(i));
			drawItemSrg->Bind(i, "_ShapeDrawList", shapeItemsBuffer.GetBuffer(i));
			drawItemSrg->Bind(i, "_LineItems", lineItemsBuffer.GetBuffer(i));
			drawItemSrg->Bind(i, "_ClipItemIndices", clipItemIndexListBuffer.GetBuffer(i));
			drawItemSrg->Bind(i, "_DrawDataConstants", drawDataConstantsBuffer[i]);
		}

		perViewSrg->FlushBindings();
		drawItemSrg->FlushBindings();
	}

	void FusionRenderer::OnBeginDestroy()
	{
		Super::OnBeginDestroy();

		drawItemsBuffer.Shutdown();
		clipItemsBuffer.Shutdown();
		shapeItemsBuffer.Shutdown();
		lineItemsBuffer.Shutdown();
		clipItemIndexListBuffer.Shutdown();

		for (int i = 0; i < numFrames; i++)
		{
			delete viewConstantsBuffer[i];
			viewConstantsBuffer[i] = nullptr;

			delete drawDataConstantsBuffer[i];
			drawDataConstantsBuffer[i] = nullptr;
		}

		for (auto drawPacket : drawPackets)
		{
			delete drawPacket;
		}
		drawPackets.Clear();

		delete perViewSrg; perViewSrg = nullptr;
		delete drawItemSrg; drawItemSrg = nullptr;
	}

	void FusionRenderer::Begin()
	{
		ZoneScoped;

		Ref<FFontManager> fontManager = FusionApplication::Get()->fontManager;

		currentBrush = FBrush();
		currentPen = FPen();
		currentFont = FFont(fontManager->GetDefaultFontFamily(), fontManager->GetDefaultFontSize());
		itemTransform = Matrix4x4::Identity();

		drawBatches.Clear();
		drawItemList.RemoveAll();
		clipItemList.RemoveAll();
		clipItemStack.RemoveAll();
		clipItemIndexList.RemoveAll();
		shapeItemList.RemoveAll();
		lineItemList.RemoveAll();
		coordinateSpaceStack.RemoveAll();

		createNewDrawBatch = true;

		for (int i = 0; i < resubmitDrawItems.GetSize(); ++i)
		{
			resubmitDrawItems[i] = true;
		}

		PushChildCoordinateSpace(Matrix4x4::Identity());
	}

	void FusionRenderer::End()
	{
		ZoneScoped;

		PopChildCoordinateSpace(); // Pop identity matrix

		auto app = FusionApplication::Get();

		const auto& vertBuffers = RPI::RPISystem::Get().GetTextQuad();
		RHI::DrawLinearArguments drawArgs = RPI::RPISystem::Get().GetTextQuadDrawArgs();

		RHI::DrawPacket* drawPacket = nullptr;

		Array<RHI::DrawPacket*> oldPackets = this->drawPackets;
		this->drawPackets.Clear();

		for (int i = 0; i < drawBatches.GetSize(); ++i)
		{
			const FDrawBatch& drawBatch = drawBatches[i];

			if (oldPackets.NotEmpty()) // Reuse draw packet
			{
				drawPacket = oldPackets[0];
				oldPackets.RemoveAt(0);

				drawPacket->drawItems[0].arguments.linearArgs.firstInstance = drawBatch.firstDrawItemIndex;
				drawPacket->drawItems[0].arguments.linearArgs.instanceCount = drawBatch.drawItemCount;

				drawPacket->shaderResourceGroups[0] = drawItemSrg;
				drawPacket->shaderResourceGroups[1] = perViewSrg;
				drawPacket->shaderResourceGroups[2] = drawBatch.fontSrg;
				//drawPacket->shaderResourceGroups[3] = app->textureSrg;

				this->drawPackets.Add(drawPacket);
			}
			else // Create new draw packet
			{
				RHI::DrawPacketBuilder builder{};

				drawArgs.firstInstance = drawBatch.firstDrawItemIndex;
				drawArgs.instanceCount = drawBatch.drawItemCount;
				builder.SetDrawArguments(drawArgs);

				builder.AddShaderResourceGroup(drawItemSrg);
				builder.AddShaderResourceGroup(perViewSrg);
				builder.AddShaderResourceGroup(drawBatch.fontSrg);
				//builder.AddShaderResourceGroup(app->textureSrg);

				// UI Item
				{
					RHI::DrawPacketBuilder::DrawItemRequest request{};
					request.vertexBufferViews.AddRange(vertBuffers);

					request.drawItemTag = drawListTag;
					request.drawFilterMask = RHI::DrawFilterMask::ALL;

					request.pipelineState = fusionShader->GetVariant(0)->GetPipeline(multisampling);

					builder.AddDrawItem(request);
				}

				drawPacket = builder.Build();
				this->drawPackets.Add(drawPacket);
			}
		}

		// Cleanup unused draw packets

		while (oldPackets.NotEmpty())
		{
			delete oldPackets[0];
			oldPackets.RemoveAt(0);
		}
	}

	///////////////////////////////////////////////
	// - Draw API -

	FusionRenderer::FDrawItem2D& FusionRenderer::DrawCustomItem(FDrawType drawType, Vec2 pos, Vec2 size)
	{
		ZoneScoped;

		Ref<FFontManager> fontManager = FusionApplication::Get()->fontManager;

		Name fontFamily = currentFont.GetFamily();
		int fontSize = currentFont.GetFontSize();

		if (fontSize <= 0)
			fontSize = 12;
		if (!fontFamily.IsValid())
			fontFamily = fontManager->GetDefaultFontFamily();

		FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
		
		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemList.GetCount();
			drawBatches.Top().fontSrg = fontAtlas != nullptr ? fontAtlas->fontSrg2 : nullptr;
		}

		FDrawItem2D drawItem{};
		drawItem.drawType = drawType;
		drawItem.quadSize = size;
		drawItem.opacity = opacityStack.IsEmpty() ? 1.0f : opacityStack.Last();

		drawItem.penColor = currentPen.GetColor().ToVec4();
		drawItem.penThickness = currentPen.GetThickness();

		switch (currentPen.GetStyle())
		{
		case FPenStyle::None:
			drawItem.penType = PEN_None;
			break;
		case FPenStyle::SolidLine:
			drawItem.penType = PEN_SolidLine;
			break;
		case FPenStyle::DashedLine:
			drawItem.penType = PEN_DashedLine;
			break;
		case FPenStyle::DottedLine:
			drawItem.penType = PEN_DottedLine;
			break;
		}

		// Translate to quad's center point first -> then apply item transform -> then translate back to top-left point of quad
		Vec3 translation1 = Vec3(-size.x / 2, -size.y / 2, 0);
		Vec3 translation2 = Vec3(pos.x + size.x / 2, pos.y + size.y / 2, 0);
		
		drawItem.transform = coordinateSpaceStack.Last() *
			Matrix4x4::Translation(translation2) *
			itemTransform *
			Matrix4x4::Translation(translation1) *
			Matrix4x4::Scale(Vec3(size.x, size.y, 1));

		for (int i = 0; i < clipItemStack.GetCount(); ++i)
		{
			int clipIndex = clipItemStack[i];
			clipItemIndexList.Insert({ .clipItemIndex = clipIndex });
		}

		if (clipItemStack.IsEmpty())
		{
			drawItem.startClipIndex = drawItem.endClipIndex = -1;
		}
		else
		{
			drawItem.startClipIndex = clipItemIndexList.GetCount() - clipItemStack.GetCount();
			drawItem.endClipIndex = clipItemIndexList.GetCount() - 1;
		}
		
		drawItemList.Insert(drawItem);

		drawBatches.Top().drawItemCount++;

		return drawItemList.Last();
	}

	Vec2 FusionRenderer::CalculateTextSize(const String& text, const FFont& font, f32 width, FWordWrap wordWrap)
	{
		static Array<Rect> offsets{};
		offsets.Reserve(512);
		return CalculateCharacterOffsets(offsets, text, font, width, wordWrap);
	}

	Vec2 FusionRenderer::CalculateCharacterOffsets(Array<Rect>& outRects, const String& text, 
		const FFont& font,
		f32 width, FWordWrap wordWrap)
	{
		ZoneScoped;

		Ref<FFontManager> fontManager = FusionApplication::Get()->fontManager;

		Name fontFamily = font.GetFamily();
		int fontSize = font.GetFontSize();

		if (fontSize <= 0)
			fontSize = fontManager->GetDefaultFontSize();
		if (!fontFamily.IsValid())
			fontFamily = fontManager->GetDefaultFontFamily();

		fontSize = Math::Max(fontSize, 6);

		const bool isFixedWidth = width > 0;

		FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
		if (fontAtlas == nullptr)
			return Vec2();

		const FFontMetrics& metrics = fontAtlas->GetMetrics();

		const float startY = metrics.ascender * (f32)fontSize;
		constexpr float startX = 0;

		float maxX = startX;
		float maxY = startY;

		Vec3 curPos = Vec3(startX, startY, 0);

		Vec2 finalSize;

		int totalCharacters = 0;
		int breakCharIdx = -1;
		int idx = 0;

		outRects.Resize(text.GetLength());

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
				curPos.y += metrics.lineHeight * (f32)fontSize;
				continue;
			}

			FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());
			
			const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize;
			const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize;

			if (isFixedWidth && curPos.x + (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize > width && wordWrap != FWordWrap::NoWrap)
			{
				curPos.x = startX;
				curPos.y += metrics.lineHeight * (f32)fontSize;

				// Go through previous characters and bring them to this new-line
				if (breakCharIdx >= 0)
				{
					for (int j = breakCharIdx + 1; j < i; j++)
					{
						char prevChar = text[j];
						FFontGlyphInfo prevGlyph = fontAtlas->FindOrAddGlyph(prevChar, fontSize, currentFont.IsBold(), currentFont.IsItalic());
						f32 atlasFontSize = prevGlyph.fontSize;

						outRects[j] = Rect::FromSize(curPos.x, curPos.y, 
							(f32)prevGlyph.GetWidth() * (f32)fontSize / (f32)prevGlyph.fontSize,
							(f32)prevGlyph.GetHeight() * (f32)fontSize / (f32)prevGlyph.fontSize);

						curPos.x += (f32)prevGlyph.advance * fontSize / atlasFontSize;
					}
					breakCharIdx = -1;

					curPos.x += (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize;
				}
				else if (wordWrap == FWordWrap::BreakWord)
				{
					breakCharIdx = -1;
					curPos.x = startX;
					curPos.y += metrics.lineHeight * fontSize;
				}
			}

			outRects[i] = Rect::FromSize(curPos.x, curPos.y, (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize, glyphHeight);

			curPos.x += (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize;

			if (curPos.x > maxX)
				maxX = curPos.x;
			if (curPos.y + metrics.lineHeight * (f32)fontSize > maxY)
				maxY = curPos.y + metrics.lineHeight * (f32)fontSize;

			totalCharacters++;
		}

		finalSize = Vec2(maxX - startX, maxY - startY);
		if (isFixedWidth)
			finalSize.width = width;
		return finalSize;
	}

	FFontMetrics FusionRenderer::GetFontMetrics(const FFont& font)
	{
		ZoneScoped;

		Ref<FFontManager> fontManager = FusionApplication::Get()->fontManager;

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

	Vec2 FusionRenderer::DrawText(const String& text, Vec2 pos, Vec2 size, FWordWrap wordWrap)
	{
		ZoneScoped;

		if (text.IsEmpty())
			return {};

		Ref<FFontManager> fontManager = FusionApplication::Get()->fontManager;

		Name fontFamily = currentFont.GetFamily();
		int fontSize = currentFont.GetFontSize();

		if (fontSize <= 0)
			fontSize = 12;
		if (!fontFamily.IsValid())
			fontFamily = fontManager->GetDefaultFontFamily();

		fontSize = Math::Max(fontSize, 6);

		const bool isFixedWidth = size.x > 0;
		const bool isFixedHeight = size.y > 0;

		FFontAtlas* fontAtlas = fontManager->FindFont(fontFamily);
		if (fontAtlas == nullptr)
			return {};

		const FFontMetrics& metrics = fontAtlas->GetMetrics();

		const float startY = pos.y + metrics.ascender * (f32)fontSize;
		const float startX = pos.x;

		if (!clipItemStack.IsEmpty())
		{
			// CPU culling

			Vec2 globalPos = coordinateSpaceStack.Last() * Vec4(pos.x, pos.y, 0, 1);
			Vec2 drawSize = size;

			if (!isFixedWidth)
			{
				drawSize = CalculateTextSize(text, currentFont, 0, wordWrap);
			}
			if (drawSize.y <= 0)
			{
				drawSize.y = metrics.lineHeight;
			}

			for (int i = clipItemStack.GetCount() - 1; i >= 0; --i)
			{
				int clipIndex = clipItemStack[i];

				Matrix4x4 clipTransform = clipItemList[clipIndex].clipTransform.GetInverse();
				Vec2 clipPos = clipTransform * Vec4(0, 0, 0, 1);

				Rect shapeRect = Rect::FromSize(globalPos, drawSize);
				Rect clipRect = Rect::FromSize(clipPos, clipItemList[clipIndex].size);

				if (!shapeRect.Overlaps(clipRect))
				{
					return {};
				}
			}
		}

		float maxX = startX;
		float maxY = startY;

		Vec3 curPos = Vec3(startX, startY, 0);

		int totalCharactersDrawn = 0;
		int firstDrawItemIndex = drawItemList.GetCount();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemList.GetCount();
			drawBatches.Top().fontSrg = fontAtlas->fontSrg2;
		}

		Vec2 finalSize;

		int breakCharIdx = -1;
		int idx = 0;

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
				curPos.y += metrics.lineHeight * fontSize;
				continue;
			}

			FFontGlyphInfo glyph = fontAtlas->FindOrAddGlyph(c, fontSize, currentFont.IsBold(), currentFont.IsItalic());
			const float glyphWidth = (f32)glyph.GetWidth() * (f32)fontSize / (f32)glyph.fontSize;
			const float glyphHeight = (f32)glyph.GetHeight() * (f32)fontSize / (f32)glyph.fontSize;

			// We are beyond the width
			if (isFixedWidth && curPos.x + (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize > startX + size.width + 1 && wordWrap != FWordWrap::NoWrap)
			{
				// Go through previous characters and bring them to this new-line
				if (breakCharIdx >= 0)
				{
					curPos.x = startX;
					curPos.y += metrics.lineHeight * (f32)fontSize;

					for (int j = breakCharIdx + 1; j < i; j++)
					{
						char prevChar = text[j];

						FFontGlyphInfo prevGlyph = fontAtlas->FindOrAddGlyph(prevChar, fontSize, currentFont.IsBold(), currentFont.IsItalic());
						f32 atlasFontSize = prevGlyph.fontSize;

						FDrawItem2D& prevDrawItem = drawItemList[firstDrawItemIndex + j];

						curPos.x += (f32)prevGlyph.xOffset * fontSize / atlasFontSize;

						Vec2 prevQuadPos = curPos;
						prevQuadPos.y -= (f32)prevGlyph.yOffset * fontSize / atlasFontSize;

						Vec3 prevTranslation = Vec3(prevQuadPos.x, prevQuadPos.y, 0);

						prevDrawItem.transform = Matrix4x4::Translation(prevTranslation) * Matrix4x4::Scale(Vec3(prevDrawItem.quadSize.x, prevDrawItem.quadSize.y, 1));

						curPos.x += (f32)prevGlyph.advance * (f32)fontSize / atlasFontSize - (f32)prevGlyph.xOffset * (f32)fontSize / atlasFontSize;

					}
					breakCharIdx = -1;
				}
				else if (wordWrap == FWordWrap::BreakWord)
				{
					breakCharIdx = -1;
					curPos.x = startX;
					curPos.y += metrics.lineHeight * fontSize;
				}
			}

			drawItemList.Insert(FDrawItem2D());

			FDrawItem2D& drawItem = drawItemList.Last();
			drawItem.drawType = DRAW_Text;
			drawItem.opacity = opacityStack.IsEmpty() ? 1.0f : opacityStack.Last();

			drawItem.penColor = currentPen.GetColor().ToVec4();
			drawItem.penThickness = currentPen.GetThickness();

			for (int j = 0; j < clipItemStack.GetCount(); ++j)
			{
				int clipIndex = clipItemStack[j];
				clipItemIndexList.Insert({ .clipItemIndex = clipIndex });
			}

			if (clipItemStack.IsEmpty())
			{
				drawItem.startClipIndex = drawItem.endClipIndex = -1;
			}
			else
			{
				drawItem.startClipIndex = clipItemIndexList.GetCount() - clipItemStack.GetCount();
				drawItem.endClipIndex = clipItemIndexList.GetCount() - 1;
			}

			drawItem.charIndex = glyph.index;

			drawItem.quadSize = Vec2(glyphWidth, glyphHeight);

			curPos.x += (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize;

			Vec2 quadPos = curPos;
			quadPos.y -= (f32)glyph.yOffset * (f32)fontSize / (f32)glyph.fontSize;

			drawItem.transform = coordinateSpaceStack.Last() *
				itemTransform *
				Matrix4x4::Translation(Vec3(quadPos.x, quadPos.y)) *
				Matrix4x4::Scale(Vec3(drawItem.quadSize.width, drawItem.quadSize.height, 1));

			curPos.x += (f32)glyph.advance * (f32)fontSize / (f32)glyph.fontSize - (f32)glyph.xOffset * (f32)fontSize / (f32)glyph.fontSize;

			if (curPos.x > maxX)
				maxX = curPos.x;
			if (curPos.y + metrics.lineHeight * (f32)fontSize > maxY)
				maxY = curPos.y + metrics.lineHeight * (f32)fontSize;

			totalCharactersDrawn++;
			idx++;
		}

		drawBatches.Top().drawItemCount += totalCharactersDrawn;

		finalSize = Vec2(maxX - startX, maxY - startY);

		return finalSize;
	}

	void FusionRenderer::DrawLine(const Vec2& from, const Vec2& to)
	{
		ZoneScoped;

		f32 lineThickness = currentPen.GetThickness();

		f32 minX = Math::Min(from.x, to.x);
		f32 minY = Math::Min(from.y, to.y);
		f32 maxX = Math::Max(from.x, to.x) + lineThickness;
		f32 maxY = Math::Max(from.y, to.y) + lineThickness;

		Vec2 size = Vec2(maxX - minX, maxY - minY);

		FDrawItem2D& drawItem = DrawCustomItem(DRAW_Line, Vec2(minX, minY), size);
		drawItem.lineIndex = lineItemList.GetCount();

		f32 dashLength = currentPen.GetDashLength();

		if (currentPen.GetStyle() == FPenStyle::DottedLine)
		{
			dashLength = 1.0f;
		}

		FLineItem2D lineItem = { .lineStart = Vec2(0, 0), .lineEnd = size, .dashLength = dashLength };

		lineItemList.Insert(lineItem);
	}

	void FusionRenderer::DrawFrameBuffer(const StaticArray<RPI::Texture*, RHI::Limits::MaxSwapChainImageCount>& frames,
		Vec2 pos, Vec2 quadSize)
	{
		if (quadSize.x <= 0 || quadSize.y <= 0)
			return;

		for (int i = 0; i < Math::Min<int>(frames.GetSize(), numFrames); ++i)
		{
			if (frames[i] == nullptr)
				return;
		}

		ZoneScoped;

		auto app = FusionApplication::Get();
		int imageIndex = -1;//app->FindImageIndex(frames[0]->GetRhiTexture());
		if (imageIndex < 0)
			return;

		FDrawItem2D& drawItem = DrawCustomItem(DRAW_FrameBuffer, pos, quadSize);

		FShapeItem2D shapeItem;
		shapeItem.shape = FShapeType::None;
		shapeItem.cornerRadius = {};
		shapeItem.textureOrGradientIndex = imageIndex;

		drawItem.shapeIndex = (uint)shapeItemList.GetCount();

		RHI::SamplerDescriptor sampler{};
		sampler.addressModeU = sampler.addressModeV = sampler.addressModeW = SamplerAddressMode::ClampToBorder;
		sampler.borderColor = SamplerBorderColor::FloatTransparentBlack;
		sampler.enableAnisotropy = false;
		sampler.samplerFilterMode = FilterMode::Linear;

		shapeItem.samplerIndex = app->FindOrCreateSampler(sampler);

		shapeItemList.Insert(shapeItem);
	}

	bool FusionRenderer::DrawShape(const FShape& shape, Vec2 pos, Vec2 quadSize)
	{
		if (quadSize.x <= 0 || quadSize.y <= 0)
			return false;

		ZoneScoped;

		if (!clipItemStack.IsEmpty())
		{
			// CPU culling

			Vec2 globalPos = coordinateSpaceStack.Last() * Vec4(pos.x, pos.y, 0, 1);

			for (int i = clipItemStack.GetCount() - 1; i >= 0; --i)
			{
				int clipIndex = clipItemStack[i];

				Matrix4x4 clipTransform = clipItemList[clipIndex].clipTransform.GetInverse();
				Vec2 clipPos = clipTransform * Vec4(0, 0, 0, 1);

				Rect shapeRect = Rect::FromSize(globalPos, quadSize);
				Rect clipRect = Rect::FromSize(clipPos, clipItemList[clipIndex].size);

				if (!shapeRect.Overlaps(clipRect))
				{
					return false;
				}
			}
		}

		FDrawItem2D& drawItem = DrawCustomItem(DRAW_Shape, pos, quadSize);
		auto app = FusionApplication::Get();

		FShapeItem2D shapeItem;
		shapeItem.shape = shape.GetShapeType();
		shapeItem.cornerRadius = shape.GetCornerRadius();

		shapeItem.brushType = BRUSH_None;

		if (currentBrush.GetBrushStyle() == FBrushStyle::SolidFill)
		{
			shapeItem.brushType = BRUSH_Solid;
			shapeItem.brushColor = currentBrush.GetFillColor().ToVec4();
		}
		else if (currentBrush.GetBrushStyle() == FBrushStyle::Image)
		{
			const Name& imageName = currentBrush.GetImageName();
			if (imageName.IsValid())
			{
				shapeItem.brushColor = currentBrush.GetFillColor().ToVec4();
			}
		}
		else if (currentBrush.GetBrushStyle() == FBrushStyle::Gradient)
		{
			shapeItem.brushType = BRUSH_LinearGradient;
		}

		switch (currentPen.GetStyle())
		{
		case FPenStyle::None:
			drawItem.penType = PEN_None;
			break;
		case FPenStyle::SolidLine:
			drawItem.penType = PEN_SolidLine;
			break;
		case FPenStyle::DashedLine:
			drawItem.penType = PEN_DashedLine;
			break;
		case FPenStyle::DottedLine:
			drawItem.penType = PEN_DottedLine;
			break;
		}

		drawItem.shapeIndex = (uint)shapeItemList.GetCount();

		shapeItemList.Insert(shapeItem);

		return true;
	}

	bool FusionRenderer::IsCulled(Vec2 pos, Vec2 quadSize)
	{
		if (!clipItemStack.IsEmpty())
		{
			// CPU culling

			Vec2 globalPos = coordinateSpaceStack.Last() * Vec4(pos.x, pos.y, 0, 1);

			for (int i = clipItemStack.GetCount() - 1; i >= 0; --i)
			{
				int clipIndex = clipItemStack[i];

				Matrix4x4 clipTransform = clipItemList[clipIndex].clipTransform.GetInverse();
				Vec2 clipPos = clipTransform * Vec4(0, 0, 0, 1);

				Rect shapeRect = Rect::FromSize(globalPos, quadSize);
				Rect clipRect = Rect::FromSize(clipPos, clipItemList[clipIndex].size);

				if (!shapeRect.Overlaps(clipRect))
				{
					return true;
				}
			}
		}

		return false;
	}

	///////////////////////////////////////////////
	// - Draw Packets -

	const Array<RHI::DrawPacket*>& FusionRenderer::FlushDrawPackets(u32 imageIndex)
	{
		ZoneScoped;

		curImageIndex = (int)imageIndex;

		drawItemGrowRatio = Math::Clamp01(drawItemGrowRatio);
		clipItemGrowRatio = Math::Clamp01(clipItemGrowRatio);
		shapeItemGrowRatio = Math::Clamp01(shapeItemGrowRatio);

		if (viewConstantsUpdateRequired[imageIndex])
		{
			viewConstantsBuffer[imageIndex]->UploadData(&viewConstants, sizeof(viewConstants));

			viewConstantsUpdateRequired[imageIndex] = false;
		}

		if (resubmitDrawItems[imageIndex])
		{
			if (drawItemList.GetCount() > 0)
			{
				if (drawItemsBuffer.GetElementCount() < drawItemList.GetCount())
				{
					u64 totalCount = (u64)((f32)drawItemsBuffer.GetElementCount() * (1.0f + drawItemGrowRatio));
					drawItemsBuffer.GrowToFit(Math::Max<u64>(drawItemList.GetCount() + 64, totalCount));

					for (int i = 0; i < numFrames; ++i)
					{
						drawItemSrg->Bind(i, "_DrawList", drawItemsBuffer.GetBuffer(i));
					}
				}

				drawItemsBuffer.GetBuffer(imageIndex)->UploadData(drawItemList.GetData(), drawItemList.GetCount() * sizeof(FDrawItem2D));
			}

			if (clipItemList.GetCount() > 0)
			{
				if (clipItemsBuffer.GetElementCount() < clipItemList.GetCount())
				{
					u64 totalCount = (u64)((f32)clipItemsBuffer.GetElementCount() * (1.0f + clipItemGrowRatio));
					clipItemsBuffer.GrowToFit(Math::Max<u64>(clipItemList.GetCount() + 32, totalCount));

					for (int i = 0; i < numFrames; ++i)
					{
						drawItemSrg->Bind(i, "_ClipItems", clipItemsBuffer.GetBuffer(i));
					}
				}

				clipItemsBuffer.GetBuffer(imageIndex)->UploadData(clipItemList.GetData(), clipItemList.GetCount() * sizeof(FClipItem2D));
			}

			if (clipItemIndexList.GetCount() > 0)
			{
				if (clipItemIndexListBuffer.GetElementCount() < clipItemIndexList.GetCount())
				{
					u64 totalCount = (u64)((f32)clipItemIndexListBuffer.GetElementCount() * (1.0f + StructuredBufferGrowRatio));
					clipItemIndexListBuffer.GrowToFit(Math::Max<u64>(clipItemIndexList.GetCount() + 16, totalCount));

					for (int i = 0; i < numFrames; ++i)
					{
						drawItemSrg->Bind(i, "_ClipItemIndices", clipItemIndexListBuffer.GetBuffer(i));
					}
				}

				clipItemIndexListBuffer.GetBuffer(imageIndex)->UploadData(clipItemIndexList.GetData(), clipItemIndexList.GetCount() * sizeof(FClipItemIndexData));
			}

			if (shapeItemList.GetCount() > 0)
			{
				if (shapeItemsBuffer.GetElementCount() < shapeItemList.GetCount())
				{
					u64 totalCount = (u64)((f32)shapeItemsBuffer.GetElementCount() * (1.0f + shapeItemGrowRatio));
					shapeItemsBuffer.GrowToFit(Math::Max<u64>(shapeItemList.GetCount() + 64, totalCount));

					for (int i = 0; i < numFrames; ++i)
					{
						drawItemSrg->Bind(i, "_ShapeDrawList", shapeItemsBuffer.GetBuffer(i));
					}
				}

				shapeItemsBuffer.GetBuffer(imageIndex)->UploadData(shapeItemList.GetData(), shapeItemList.GetCount() * sizeof(FShapeItem2D));
			}

			if (lineItemList.GetCount() > 0)
			{
				if (lineItemsBuffer.GetElementCount() < lineItemList.GetCount())
				{
					u64 totalCount = (u64)((f32)lineItemsBuffer.GetElementCount() * (1.0f + StructuredBufferGrowRatio));
					lineItemsBuffer.GrowToFit(Math::Max<u64>(lineItemList.GetCount() + 64, totalCount));

					for (int i = 0; i < numFrames; ++i)
					{
						drawItemSrg->Bind(i, "_LineItems", lineItemsBuffer.GetBuffer(i));
					}
				}

				lineItemsBuffer.GetBuffer(imageIndex)->UploadData(lineItemList.GetData(), lineItemList.GetCount() * sizeof(FLineItem2D));
			}

			resubmitDrawItems[imageIndex] = false;
		}

		drawItemSrg->FlushBindings();

		return drawPackets;
	}

	void FusionRenderer::SetViewConstants(const RPI::PerViewConstants& viewConstants)
	{
		this->viewConstants = viewConstants;

		for (int i = 0; i < viewConstantsUpdateRequired.GetSize(); ++i)
		{
			viewConstantsUpdateRequired[i] = true;
		}
	}

	void FusionRenderer::SetItemTransform(const Matrix4x4& transform)
	{
		this->itemTransform = transform;
	}

	void FusionRenderer::SetBrush(const FBrush& brush)
	{
		currentBrush = brush;
	}

	void FusionRenderer::SetPen(const FPen& pen)
	{
		currentPen = pen;
	}

	void FusionRenderer::SetFont(const FFont& font)
	{
		Name fontFamily = font.GetFamily();
		if (!fontFamily.IsValid())
		{
			fontFamily = FusionApplication::Get()->GetFontManager()->GetDefaultFontFamily();
		}
		Name curFontFamily = currentFont.GetFamily();
		if (!curFontFamily.IsValid())
		{
			curFontFamily = FusionApplication::Get()->GetFontManager()->GetDefaultFontFamily();
		}

		if (curFontFamily != fontFamily)
		{
			createNewDrawBatch = true;
		}

		currentFont = font;
	}

	void FusionRenderer::PushOpacity(f32 opacity)
	{
		if (opacityStack.IsEmpty())
		{
			opacityStack.Insert(opacity);
		}
		else
		{
			opacityStack.Insert(opacityStack.Last() * opacity);
		}
	}

	void FusionRenderer::PopOpacity()
	{
		if (!opacityStack.IsEmpty())
		{
			opacityStack.RemoveLast();
		}
	}

	void FusionRenderer::SetScreenSize(Vec2i screenSize)
	{
		this->screenSize = screenSize;
	}

	void FusionRenderer::SetDrawListTag(RHI::DrawListTag drawListTag)
	{
		this->drawListTag = drawListTag;
	}

	u64 FusionRenderer::ComputeMemoryFootprint()
	{
		u64 memory = Super::ComputeMemoryFootprint();

		memory += shapeItemList.GetCapacity() * sizeof(FShapeItem2D);
		memory += drawItemList.GetCapacity() * sizeof(FDrawItem2D);
		memory += clipItemList.GetCapacity() * sizeof(FClipItem2D);
		memory += clipItemStack.GetCapacity() * sizeof(int);
		memory += clipItemIndexList.GetCapacity() * sizeof(FClipItemIndexData);
		memory += coordinateSpaceStack.GetCapacity() * sizeof(Matrix4x4);
		memory += opacityStack.GetCapacity() * sizeof(f32);
		memory += lineItemList.GetCapacity() * sizeof(FLineItem2D);
		memory += drawBatches.GetSize() * sizeof(FDrawBatch);
		memory += drawPackets.GetSize() * sizeof(DrawPacket);

		memory += drawItemsBuffer.GetBuffer(0)->GetBufferSize() * 2;
		memory += clipItemsBuffer.GetBuffer(0)->GetBufferSize() * 2;
		memory += shapeItemsBuffer.GetBuffer(0)->GetBufferSize() * 2;
		memory += lineItemsBuffer.GetBuffer(0)->GetBufferSize() * 2;
		memory += clipItemIndexListBuffer.GetBuffer(0)->GetBufferSize() * 2;

		return memory;
	}

	void FusionRenderer::PushChildCoordinateSpace(const Matrix4x4& transform)
	{
		if (coordinateSpaceStack.IsEmpty())
		{
			coordinateSpaceStack.Insert(transform);
		}
		else
		{
			coordinateSpaceStack.Insert(coordinateSpaceStack.Last() * transform);
		}
	}

	const Matrix4x4& FusionRenderer::GetTopCoordinateSpace()
	{
		if (coordinateSpaceStack.IsEmpty())
		{
			static Matrix4x4 identity = Matrix4x4::Identity();
			return identity;
		}

		return coordinateSpaceStack.Last();
	}

	void FusionRenderer::PopChildCoordinateSpace()
	{
		if (!coordinateSpaceStack.IsEmpty())
		{
			coordinateSpaceStack.RemoveLast();
		}
	}

	void FusionRenderer::PushClipShape(const Matrix4x4& clipTransform, Vec2 rectSize, const FShape& shape)
	{
		Matrix4x4 inverse = (coordinateSpaceStack.Last() * clipTransform).GetInverse();

		// Clipping is done via SDF functions, which require you to inverse the transformations applied
		clipItemList.Insert(FClipItem2D{
				.clipTransform = inverse,
				.cornerRadius = shape.GetCornerRadius(),
				.size = rectSize,
				.shapeType = shape.GetShapeType()
			});

		clipItemStack.Insert(clipItemList.GetCount() - 1);
	}

	void FusionRenderer::PopClipShape()
	{
		clipItemStack.RemoveLast();
	}

} // namespace CE
