#include "CoreRPI.h"

namespace CE::RPI
{
	constexpr bool ForceDisableBatching = false;

	SIZE_T ColorGradient::Key::GetHash() const
	{
		SIZE_T hash = CE::GetHash(color);
		CombineHash(hash, position);
		return hash;
	}

	SIZE_T ColorGradient::GetHash() const
	{
		SIZE_T hash = CE::GetHash(degrees);

		for (int i = 0; i < keys.GetSize(); ++i)
		{
			CombineHash(hash, keys[i]);
		}

		return hash;
	}

	Renderer2D::Renderer2D(const Renderer2DDescriptor& desc)
		: screenSize(desc.screenSize)
		, drawShader(desc.drawShader)
		, multisampling(desc.multisampling)
		, drawListTag(desc.drawListTag)
		, numFramesInFlight(desc.numFramesInFlight)
		, initialDrawItemStorage(desc.initialDrawItemStorage)
		, drawItemStorageIncrement(desc.drawItemStorageIncrement)
		, viewConstants(desc.viewConstantData)
		, rootTransform(desc.rootTransform)
	{
		CE_ASSERT(drawShader != nullptr, "Draw Shader is null");
		CE_ASSERT(screenSize.x > 0 && screenSize.y > 0, "Screen size is zero!");

		numFramesInFlight = Math::Min(numFramesInFlight, RHI::Limits::MaxSwapChainImageCount);

		textQuadVertices = RPISystem::Get().GetTextQuad();
		textQuadDrawArgs = RPISystem::Get().GetTextQuadDrawArgs();

		defaultMaterial = new RPI::Material(drawShader);

		ShaderVariant* drawShaderVariant = drawShader->GetVariant(drawShader->GetDefaultVariantIndex());

		RHI::ShaderResourceGroupDescriptor perViewSrgDesc{};
		perViewSrgDesc.name = "SRG_PerView";
		perViewSrgDesc.layout = drawShaderVariant->GetSrgLayout(RHI::SRGType::PerView);

		perViewSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perViewSrgDesc);

		RHI::ShaderResourceGroupDescriptor perDrawSrgDesc{};
		perDrawSrgDesc.name = "SRG_PerDraw";
		perDrawSrgDesc.layout = drawShaderVariant->GetSrgLayout(RHI::SRGType::PerDraw);

		drawItemSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perDrawSrgDesc);

		for (int i = 0; i < numFramesInFlight; i++)
		{
			RHI::BufferDescriptor drawItemBufferDesc{};
			drawItemBufferDesc.name = "DrawItem Buffer";
			drawItemBufferDesc.bufferSize = initialDrawItemStorage * sizeof(DrawItem2D);
			drawItemBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			drawItemBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer;
			drawItemBufferDesc.structureByteStride = sizeof(DrawItem2D);

			drawItemsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(drawItemBufferDesc);
			drawItemSrg->Bind(i, "_DrawList", drawItemsBuffer[i]);

			RHI::BufferDescriptor clipRectBufferDesc{};
			clipRectBufferDesc.name = "ClipRect Buffer";
			clipRectBufferDesc.bufferSize = 100 * sizeof(ClipRect2D);
			clipRectBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			clipRectBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer;
			clipRectBufferDesc.structureByteStride = sizeof(ClipRect2D);

			clipRectsBuffer[i] = gDynamicRHI->CreateBuffer(clipRectBufferDesc);
			drawItemSrg->Bind(i, "_ClipRects", clipRectsBuffer[i]);

			RHI::BufferDescriptor gradientKeysBufferDesc{};
			gradientKeysBufferDesc.name = "ClipRect Buffer";
			gradientKeysBufferDesc.bufferSize = 50 * sizeof(ColorGradient::Key);
			gradientKeysBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			gradientKeysBufferDesc.bindFlags = BufferBindFlags::StructuredBuffer;
			gradientKeysBufferDesc.structureByteStride = sizeof(ColorGradient::Key);

			gradientKeysBuffer[i] = gDynamicRHI->CreateBuffer(gradientKeysBufferDesc);
			drawItemSrg->Bind(i, "_GradientKeys", gradientKeysBuffer[i]);

			RHI::BufferDescriptor viewConstantsBufferDesc{};
			viewConstantsBufferDesc.name = "ViewConstants";
			viewConstantsBufferDesc.bufferSize = sizeof(viewConstants);
			viewConstantsBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			viewConstantsBufferDesc.bindFlags = BufferBindFlags::ConstantBuffer;
			viewConstantsBufferDesc.structureByteStride = sizeof(viewConstants);

			viewConstantsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(viewConstantsBufferDesc);
			viewConstantsBuffer[i]->UploadData(&viewConstants, sizeof(viewConstants));
			perViewSrg->Bind(i, "_PerViewData", viewConstantsBuffer[i]);

			DrawDataConstants drawDataConstants{};
			drawDataConstants.frameIndex = i;

			RHI::BufferDescriptor drawDataConstantsBufferDesc{};
			drawDataConstantsBufferDesc.name = "DrawDataConstants";
			drawDataConstantsBufferDesc.bufferSize = sizeof(DrawDataConstants);
			drawDataConstantsBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			drawDataConstantsBufferDesc.bindFlags = BufferBindFlags::ConstantBuffer;
			drawDataConstantsBufferDesc.structureByteStride = sizeof(DrawDataConstants);

			drawDataConstantsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(drawDataConstantsBufferDesc);
			drawDataConstantsBuffer[i]->UploadData(&drawDataConstants, sizeof(drawDataConstants));
			drawItemSrg->Bind(i, "_DrawDataConstants", drawDataConstantsBuffer[i]);
		}

		{
			RHI::SamplerDescriptor textureSampler{};
			textureSampler.borderColor = SamplerBorderColor::FloatTransparentBlack;
			textureSampler.enableAnisotropy = false;
			textureSampler.samplerFilterMode = FilterMode::Cubic;

			textureSampler.addressModeU = textureSampler.addressModeV = textureSampler.addressModeW = SamplerAddressMode::ClampToBorder;
			samplerDescriptors.Add(textureSampler);

			textureSampler.addressModeU = SamplerAddressMode::Repeat;
			textureSampler.addressModeV = SamplerAddressMode::ClampToBorder;
			samplerDescriptors.Add(textureSampler);

			textureSampler.addressModeU = SamplerAddressMode::ClampToBorder;
			textureSampler.addressModeV = SamplerAddressMode::Repeat;
			samplerDescriptors.Add(textureSampler);

			textureSampler.addressModeU = SamplerAddressMode::Repeat;
			textureSampler.addressModeV = SamplerAddressMode::Repeat;
			samplerDescriptors.Add(textureSampler);
		}

		{
			samplers.Reserve(samplerDescriptors.GetSize());
			samplerIndices.Clear();

			for (int i = 0; i < samplerDescriptors.GetSize(); i++)
			{
				samplerIndices[samplerDescriptors[i]] = i;
				samplers.Add(RPISystem::Get().FindOrCreateSampler(samplerDescriptors[i]));
			}

			drawItemSrg->Bind("_TextureSamplers", samplers.GetSize(), samplers.GetData());
		}
		
		perViewSrg->FlushBindings();
		drawItemSrg->FlushBindings();
	}

	Renderer2D::~Renderer2D()
	{
		for (int i = 0; i < numFramesInFlight; i++)
		{
			delete drawItemsBuffer[i];
			drawItemsBuffer[i] = nullptr;

			delete clipRectsBuffer[i];
			clipRectsBuffer[i] = nullptr;

			delete viewConstantsBuffer[i];
			viewConstantsBuffer[i] = nullptr;

			delete drawDataConstantsBuffer[i];
			drawDataConstantsBuffer[i] = nullptr;

			delete gradientKeysBuffer[i];
			gradientKeysBuffer[i] = nullptr;
		}

		for (const auto& [variant, material] : materials)
		{
			delete material;
		}
		materials.Clear();

		delete defaultMaterial;

		for (auto drawPacket : drawPackets)
		{
			delete drawPacket;
		}
		drawPackets.Clear();

		delete perViewSrg; perViewSrg = nullptr;
		delete drawItemSrg; drawItemSrg = nullptr;
	}

	void Renderer2D::RegisterFont(Name name, RPI::FontAtlasAsset* fontAtlasData)
	{
		if (!name.IsValid() || fontAtlasData == nullptr)
			return;

		if (defaultFont == nullptr)
		{
			defaultFontName = name;
			defaultFont = fontAtlasData;
		}

		fontAtlasesByName[name] = fontAtlasData;
	}

	void Renderer2D::SetScreenSize(Vec2i newScreenSize)
	{
		this->screenSize = newScreenSize;
	}

	void Renderer2D::Begin()
	{
		ZoneScoped;

		fontStack.Push({ .fontName = defaultFontName, .fontSize = 16});

		drawBatches.Clear();
		clipRectStack.Clear();
		textureIndices.Clear();
		gradientIndices.Clear();

		drawItemCount = 0;
		clipRectCount = 0;
		textureCount = 0;
		gradientKeyCount = 0;
		createNewDrawBatch = true;

		ResetToDefaults();

		PushClipRect(Rect::FromSize(Vec2(0, 0), screenSize.ToVec2()));

		for (int i = 0; i < resubmitDrawData.GetSize(); i++)
		{
			resubmitDrawData[i] = true;
			resubmitClipRects[i] = true;
			resubmitGradientKeys[i] = true;
		}
	}

	void Renderer2D::ResetToDefaults()
	{
		rotation = 0.0;
		borderThickness = 0.0f;
		fillColor = Color(1, 1, 1, 1);
		outlineColor = Color(0, 0, 0, 0);
	}

	void Renderer2D::PushFont(Name family, u32 fontSize, bool bold)
	{
		if (!family.IsValid())
		{
			family = fontStack.Top().fontName;
		}

		if (fontStack.Top().fontName != family)
			createNewDrawBatch = true;

		fontStack.Push({ .fontName = family, .fontSize = fontSize, .bold = bold });
	}

	void Renderer2D::PopFont()
	{
		const FontInfo& last = fontStack.Top();
		const FontInfo& lastSecond = fontStack[fontStack.GetSize() - 2];

		if (last.fontName != lastSecond.fontName)
			createNewDrawBatch = true;

		fontStack.Pop();
	}

	void Renderer2D::PushClipRect(Rect clipRect, Vec4 cornerRadius)
	{
		if (clipRects.GetSize() < clipRectCount + 1)
		{
			clipRects.Resize(clipRectCount + 1);
		}

		if (clipRectStack.NotEmpty())
		{
			clipRect.min.y = Math::Max(clipRect.min.y, clipRects[clipRectStack.Top()].rect.min.y);
			clipRect.max.y = Math::Min(clipRect.max.y, clipRects[clipRectStack.Top()].rect.max.y);
			clipRect.min.x = Math::Max(clipRect.min.x, clipRects[clipRectStack.Top()].rect.min.x);
			clipRect.max.x = Math::Min(clipRect.max.x, clipRects[clipRectStack.Top()].rect.max.x);
		}

		clipRects[clipRectCount] = { .rect = clipRect, .cornerRadius = cornerRadius };
		clipRectStack.Push(clipRectCount);
		clipRectCount++;
	}

	void Renderer2D::PopClipRect()
	{
		if (clipRectStack.NotEmpty())
			clipRectStack.Pop();
	}

	bool Renderer2D::ClipRectExists()
	{
		return clipRectStack.NotEmpty();
	}

	Rect Renderer2D::GetLastClipRect()
	{
		if (!ClipRectExists())
			return Rect();
		return clipRects[clipRectStack.Top()].rect;
	}

	void Renderer2D::SetFillGradient(const ColorGradient& gradient, GradientType gradientType)
	{
		if (gradient.keys.GetSize() < 2)
		{
			currentGradientType = GradientType::None;
			currentGradient = Vec2i();
			return;
		}

		currentGradientType = gradientType;
		gradientDegrees = gradient.degrees;

		if (gradientIndices.KeyExists(gradient))
		{
			currentGradient = gradientIndices[gradient];
			return;
		}

		if (gradientKeys.GetSize() < gradientKeyCount + gradient.keys.GetSize())
			gradientKeys.Resize(gradientKeyCount + gradient.keys.GetSize());

		for (int i = 0; i < gradient.keys.GetSize(); ++i)
		{
			gradientKeys[gradientKeyCount + i] = gradient.keys[i];
		}

		gradientIndices[gradient] = { (int)gradientKeyCount, (int)(gradientKeyCount + gradient.keys.GetSize() - 1) };
		currentGradient = gradientIndices[gradient];

		gradientKeyCount += gradient.keys.GetSize();
	}

	void Renderer2D::SetFillColor(const Color& color)
	{
		this->fillColor = color;
	}

	void Renderer2D::SetOutlineColor(const Color& color)
	{
		this->outlineColor = color;
	}

	void Renderer2D::SetBorderThickness(f32 thickness)
	{
		this->borderThickness = thickness;
	}

	void Renderer2D::SetRotation(f32 degrees)
	{
		this->rotation = degrees;
	}

	void Renderer2D::SetCursor(Vec2 position)
	{
		this->cursorPosition = position;
	}

	Vec2 Renderer2D::CalculateTextSize(const String& text, f32 width)
	{
		Vec2 size{};
		size.width = width;

		const bool isFixedWidth = width > 0;

		const FontInfo& font = fontStack.Top();

		RPI::FontAtlasAsset* fontAtlas = fontAtlasesByName[font.fontName];
		if (fontAtlas == nullptr)
			return Vec2();

		const auto& metrics = fontAtlas->GetMetrics();
		f32 fontSize = font.fontSize;
		f32 atlasFontSize = metrics.fontSize;

		const float startY = cursorPosition.y + metrics.ascender * fontSize / atlasFontSize;
		const float startX = cursorPosition.x;

		float maxX = startX;
		float maxY = startY;

		Vec3 position = Vec3(startX, startY, 0);
		int whitespaceIdx = -1;

		struct CharacterPosition
		{
			Vec3 translation;
			Vec3 scale;
		};

		static Array<CharacterPosition> positions{};
		if (positions.GetSize() < text.GetLength())
		{
			positions.Resize(text.GetLength());
		}

		for (int i = 0; i < text.GetLength(); i++)
		{
			char c = text[i];

			if (c == ' ')
			{
				whitespaceIdx = i;
			}

			if (c == '\n')
			{
				whitespaceIdx = -1;
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;
				continue;
			}

			const RPI::FontGlyphLayout& glyphLayout = fontAtlas->GetGlyphLayout(c);

			const float glyphWidth = (f32)glyphLayout.GetWidth();
			const float glyphHeight = (f32)glyphLayout.GetHeight();

			position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			if (isFixedWidth && position.x + glyphWidth * fontSize / atlasFontSize > width)
			{
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;

				// Go through previous characters and bring them to this new-line
				if (whitespaceIdx >= 0)
				{
					position.x -= (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

					for (int j = whitespaceIdx + 1; j < i; j++)
					{
						char prevChar = text[j];
						const RPI::FontGlyphLayout& prevGlyphLayout = fontAtlas->GetGlyphLayout(prevChar);

						position.x += (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

						position.x += (f32)prevGlyphLayout.advance * fontSize / atlasFontSize - (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

					}
					whitespaceIdx = -1;

					position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;
				}
			}

			positions[i] = { .translation = position, .scale = Vec3((f32)glyphLayout.advance * fontSize / atlasFontSize, 0, 0) };

			position.x += (f32)glyphLayout.advance * fontSize / atlasFontSize - (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			if (position.x > maxX)
				maxX = position.x;
			if (position.y + metrics.lineHeight * fontSize / atlasFontSize > maxY)
				maxY = position.y + metrics.lineHeight * fontSize / atlasFontSize;
		}

		size = Vec2(maxX - startX, maxY - startY);
		if (isFixedWidth)
			size.width = width;
		return size;
	}

	Vec2 Renderer2D::CalculateTextSize(const String& text, f32 fontSize, Name fontName, f32 width)
	{
		Vec2 size{};
		size.width = width;

		const bool isFixedWidth = width > 0;
		if (!fontName.IsValid())
			fontName = defaultFontName;
		
		RPI::FontAtlasAsset* fontAtlas = fontAtlasesByName[fontName];
		if (fontAtlas == nullptr)
			return Vec2();

		const auto& metrics = fontAtlas->GetMetrics();
		f32 atlasFontSize = metrics.fontSize;

		const float startY = cursorPosition.y + metrics.ascender * fontSize / atlasFontSize;
		const float startX = cursorPosition.x;

		float maxX = startX;
		float maxY = startY;

		Vec3 position = Vec3(startX, startY, 0);
		int whitespaceIdx = -1;

		struct CharacterPosition
		{
			Vec3 translation;
			Vec3 scale;
		};

		static Array<CharacterPosition> positions{};
		if (positions.GetSize() < text.GetLength())
		{
			positions.Resize(text.GetLength());
		}

		for (int i = 0; i < text.GetLength(); i++)
		{
			char c = text[i];

			if (c == ' ')
			{
				whitespaceIdx = i;
			}

			if (c == '\n')
			{
				whitespaceIdx = -1;
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;
				continue;
			}

			const RPI::FontGlyphLayout& glyphLayout = fontAtlas->GetGlyphLayout(c);

			const float glyphWidth = (f32)glyphLayout.GetWidth();
			const float glyphHeight = (f32)glyphLayout.GetHeight();

			position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			if (isFixedWidth && position.x + glyphWidth * fontSize / atlasFontSize > startX + width)
			{
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;

				// Go through previous characters and bring them to this new-line
				if (whitespaceIdx >= 0)
				{
					position.x -= (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

					for (int j = whitespaceIdx + 1; j < i; j++)
					{
						char prevChar = text[j];
						const RPI::FontGlyphLayout& prevGlyphLayout = fontAtlas->GetGlyphLayout(prevChar);

						position.x += (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

						position.x += (f32)prevGlyphLayout.advance * fontSize / atlasFontSize - (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

					}
					whitespaceIdx = -1;

					position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;
				}
			}

			positions[i] = { .translation = position, .scale = Vec3((f32)glyphLayout.advance * fontSize / atlasFontSize, 0, 0) };

			position.x += (f32)glyphLayout.advance * fontSize / atlasFontSize - (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			if (position.x > maxX)
				maxX = position.x;
			if (position.y + metrics.lineHeight * fontSize / atlasFontSize > maxY)
				maxY = position.y + metrics.lineHeight * fontSize / atlasFontSize;
		}

		size = Vec2(maxX - startX, maxY - startY);
		if (isFixedWidth)
			size.width = width;
		return size;
	}

	Vec2 Renderer2D::CalculateTextOffsets(Array<Rect>& outOffsetRects, const String& text, f32 width)
	{
		const FontInfo& font = fontStack.Top();

		return CalculateTextOffsets(outOffsetRects, text, font.fontSize, font.fontName, width);
	}

	Vec2 Renderer2D::CalculateTextOffsets(Array<Rect>& outRects, const String& text, f32 fontSize, Name fontName, f32 width)
	{
		Vec2 size{};
		size.width = width;

		const bool isFixedWidth = width > 0;
		if (!fontName.IsValid())
			fontName = defaultFontName;

		RPI::FontAtlasAsset* fontAtlas = fontAtlasesByName[fontName];
		if (fontAtlas == nullptr)
			return Vec2();

		const auto& metrics = fontAtlas->GetMetrics();
		f32 atlasFontSize = metrics.fontSize;

		const float startY = cursorPosition.y;// + metrics.ascender * fontSize / atlasFontSize;
		const float startX = cursorPosition.x;

		float maxX = startX;
		float maxY = startY;

		Vec3 position = Vec3(startX, startY, 0);
		int whitespaceIdx = -1;
		outRects.Resize(text.GetLength());

		for (int i = 0; i < text.GetLength(); i++)
		{
			char c = text[i];

			if (c == ' ')
			{
				whitespaceIdx = i;
			}

			if (c == '\n')
			{
				whitespaceIdx = -1;
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;
				continue;
			}

			const RPI::FontGlyphLayout& glyphLayout = fontAtlas->GetGlyphLayout(c);

			const float glyphWidth = (f32)glyphLayout.GetWidth();
			const float glyphHeight = (f32)glyphLayout.GetHeight();

			position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			outRects[i].min = position - cursorPosition;
			outRects[i].max = outRects[i].min + Vec2(glyphWidth * fontSize / atlasFontSize, glyphHeight * fontSize / atlasFontSize);

			if (abs(glyphWidth) < 0.00001f)
			{
				// TODO: Temporary hack
				const FontGlyphLayout& hLayout = fontAtlas->GetGlyphLayout('h');

				outRects[i].max = outRects[i].min + Vec2((f32)glyphLayout.advance * fontSize / atlasFontSize, hLayout.GetHeight() * fontSize / atlasFontSize);
			}

			if (isFixedWidth && position.x + glyphWidth * fontSize / atlasFontSize > startX + width)
			{
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;

				// Go through previous characters and bring them to this new-line
				if (whitespaceIdx >= 0)
				{
					position.x -= (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

					for (int j = whitespaceIdx + 1; j < i; j++)
					{
						char prevChar = text[j];
						const RPI::FontGlyphLayout& prevGlyphLayout = fontAtlas->GetGlyphLayout(prevChar);

						position.x += (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

						outRects[j].min = position - cursorPosition;
						outRects[j].max = outRects[j].min + Vec2(glyphWidth * fontSize / atlasFontSize, glyphHeight * fontSize / atlasFontSize);

						position.x += (f32)prevGlyphLayout.advance * fontSize / atlasFontSize - (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

					}
					whitespaceIdx = -1;

					position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;
				}
			}

			position.x += (f32)glyphLayout.advance * fontSize / atlasFontSize - (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			if (position.x > maxX)
				maxX = position.x;
			if (position.y + metrics.lineHeight * fontSize / atlasFontSize > maxY)
				maxY = position.y + metrics.lineHeight * fontSize / atlasFontSize;
		}

		size = Vec2(maxX - startX, maxY - startY);
		if (isFixedWidth)
			size.width = width;
		return size;
	}

	f32 Renderer2D::GetFontLineHeight()
	{
		const FontInfo& font = fontStack.Top();
		Name fontName = font.fontName;

		if (!fontName.IsValid())
			fontName = defaultFontName;

		RPI::FontAtlasAsset* fontAtlas = fontAtlasesByName[fontName];
		if (fontAtlas == nullptr)
			return 0;

		const auto& metrics = fontAtlas->GetMetrics();
		f32 atlasFontSize = metrics.fontSize;

		return metrics.lineHeight * (f32)font.fontSize / atlasFontSize;
	}

	Vec2 Renderer2D::DrawText(const String& text, Vec2 size)
	{
		if (text.IsEmpty())
			return Vec2();

		const bool isFixedWidth = size.width > 0;
		const bool isFixedHeight = size.height > 0;

		const FontInfo& font = fontStack.Top();

		RPI::FontAtlasAsset* fontAtlas = fontAtlasesByName[font.fontName];
		if (fontAtlas == nullptr)
			return Vec2();

		if (drawItems.GetSize() < drawItemCount + text.GetLength())
			drawItems.Resize(drawItemCount + text.GetLength());

		const auto& metrics = fontAtlas->GetMetrics();
		f32 fontSize = font.fontSize;
		f32 atlasFontSize = metrics.fontSize;
		
		const float startY = cursorPosition.y + metrics.ascender * fontSize / atlasFontSize;
		const float startX = cursorPosition.x;

		float maxX = startX;
		float maxY = startY;

		Vec3 position = Vec3(startX, startY, 0);

		int totalCharactersDrawn = 0;
		int firstDrawItemIndex = drawItemCount;

		Matrix4x4 rotationMat = Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		DrawBatch& curDrawBatch = drawBatches.Top();
		int whitespaceIdx = -1;
		int idx = 0;

		for (int i = 0; i < text.GetLength(); i++)
		{
			char c = text[i];

			if (c == ' ')
			{
				whitespaceIdx = i;
			}

			if (c == '\n')
			{
				whitespaceIdx = -1;
				position.x = startX;
				position.y += metrics.lineHeight * fontSize / atlasFontSize;
				continue;
			}

			DrawItem2D& drawItem = drawItems[firstDrawItemIndex + totalCharactersDrawn];
			
			const RPI::FontGlyphLayout& glyphLayout = fontAtlas->GetGlyphLayout(c);

			Vec3 scale = Vec3(1, 1, 1);
			const float glyphWidth = (f32)glyphLayout.GetWidth();
			const float glyphHeight = (f32)glyphLayout.GetHeight();

			// Need to multiply by 2 because final range is [-w, w] instead of [0, w] (which is double the size)
			scale.x = glyphWidth * fontSize / atlasFontSize * 2;
			scale.y = glyphHeight * fontSize / atlasFontSize * 2;

			position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

			if (isFixedWidth && position.x + glyphWidth * fontSize / atlasFontSize > startX + size.width)
			{
				// Go through previous characters and bring them to this new-line
				if (whitespaceIdx >= 0)
				{
					position.x = startX;
					position.y += metrics.lineHeight * fontSize / atlasFontSize;

					position.x -= (f32)glyphLayout.xOffset * fontSize / atlasFontSize;

					for (int j = whitespaceIdx + 1; j < i; j++)
					{
						char prevChar = text[j];

						const RPI::FontGlyphLayout& prevGlyphLayout = fontAtlas->GetGlyphLayout(prevChar);

						DrawItem2D& prevDrawItem = drawItems[firstDrawItemIndex + j];

						position.x += (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;

						Vec2 prevQuadPos = position;
						prevQuadPos.y -= (f32)prevGlyphLayout.yOffset * fontSize / atlasFontSize;

						// Need to multiply by 2 because final range is [-w, w] instead of [0, w] (which is double the size)
						Vec3 prevTranslation = Vec3(prevQuadPos.x * 2, prevQuadPos.y * 2, 0);
						
						prevDrawItem.transform = rotationMat * Matrix4x4::Translation(prevTranslation) * Matrix4x4::Scale(prevDrawItem.itemSize);

						position.x += (f32)prevGlyphLayout.advance * fontSize / atlasFontSize - (f32)prevGlyphLayout.xOffset * fontSize / atlasFontSize;
						
					}
					whitespaceIdx = -1;

					position.x += (f32)glyphLayout.xOffset * fontSize / atlasFontSize;
				}
			}
			
			Vec2 quadPos = position;
			quadPos.y -= (f32)glyphLayout.yOffset * fontSize / atlasFontSize;

			// Need to multiply by 2 because final range is [-w, w] instead of [0, w] (which is double the size)
			Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);
			
			drawItem.fillColor = outlineColor.ToVec4();
			drawItem.outlineColor = outlineColor.ToVec4();
			drawItem.itemSize = scale;
			drawItem.borderThickness = 0.0f;
			drawItem.bold = font.bold ? 1 : 0;
			drawItem.drawType = DRAW_Text;
			drawItem.clipRectIdx = clipRectStack.Top();
			
			drawItem.transform = rotationMat * Matrix4x4::Translation(translation) * Matrix4x4::Scale(scale);
			drawItem.charIndex = fontAtlas->GetCharacterIndex(c);

			position.x += (f32)glyphLayout.advance * fontSize / atlasFontSize - (f32)glyphLayout.xOffset * fontSize / atlasFontSize;
			
			if (position.x > maxX)
				maxX = position.x;
			if (position.y + metrics.lineHeight * fontSize / atlasFontSize > maxY)
				maxY = position.y + metrics.lineHeight * fontSize / atlasFontSize;

			totalCharactersDrawn++;
			idx++;
		}

		drawItemCount += totalCharactersDrawn;
		curDrawBatch.drawItemCount += totalCharactersDrawn;

		size = Vec2(maxX - startX, maxY - startY);
		return size;
	}

	Vec2 Renderer2D::DrawCircle(Vec2 size)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];
		
		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation) * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_Circle;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = outlineColor.ToVec4();
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();

		if (currentGradientType == GradientType::Linear && currentGradient.x < currentGradient.y)
		{
			drawItem.gradientStartIndex = currentGradient.x;
			drawItem.gradientEndIndex = currentGradient.y;
			drawItem.gradientRadians = gradientDegrees * DEG_TO_RAD;
		}

		curDrawBatch.drawItemCount++;
		
		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawRect(Vec2 size)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];
		drawItem = {};

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		Vec3 translation1 = Vec3(-scale.x / 2, -scale.y / 2);
		Vec3 translation2 = Vec3(quadPos.x * 2 + scale.x / 2, quadPos.y * 2 + scale.y / 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation2) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() *
			Matrix4x4::Translation(translation1) * Matrix4x4::Scale(scale);

		//drawItem.transform = Matrix4x4::Translation(translation) * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_Rect;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = outlineColor.ToVec4();
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();

		if (currentGradientType == GradientType::Linear && currentGradient.x < currentGradient.y)
		{
			drawItem.gradientStartIndex = currentGradient.x;
			drawItem.gradientEndIndex = currentGradient.y;
			drawItem.gradientRadians = gradientDegrees * DEG_TO_RAD;
		}

		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawRoundedRect(Vec2 size, const Vec4& cornerRadius)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];
		drawItem = {};

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_RoundedRect;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = outlineColor.ToVec4();
		drawItem.cornerRadius = cornerRadius;
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();

		if (currentGradientType == GradientType::Linear && currentGradient.x < currentGradient.y)
		{
			drawItem.gradientStartIndex = currentGradient.x;
			drawItem.gradientEndIndex = currentGradient.y;
			drawItem.gradientRadians = gradientDegrees * DEG_TO_RAD;
		}

		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawDashedLine(Vec2 size, f32 dashLength)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		DrawItem2D& drawItem = DrawCustomItem(DRAW_DashedLine, size);
		drawItem.dashLength = dashLength;
		
		return drawItem.itemSize;
	}

	Vec2 Renderer2D::DrawTriangle(Vec2 size)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];
		drawItem = {};

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * Matrix4x4::Scale(scale);
		//drawItem.transform = Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * Matrix4x4::Translation(translation) * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_Triangle;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = outlineColor.ToVec4();
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();

		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawRoundedX(Vec2 size)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_RoundedX;
		drawItem.fillColor = outlineColor.ToVec4();
		drawItem.outlineColor = outlineColor.ToVec4();
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();
		
		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawTexture(RPI::Texture* texture, Vec2 size)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		int textureIndex = 0;

		if (textureIndices.KeyExists(texture))
		{
			textureIndex = textureIndices[texture];
		}
		else
		{
			textureIndex = textureCount;
			if (textures.GetSize() < textureCount + 1)
				textures.Resize(textureCount + 1);

			textureIndices[texture] = textureIndex;

			textureCount++;
		}
		
		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];

		textures[textureIndex] = texture;

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation1 = Vec3(-scale.x / 2, -scale.y / 2);
		Vec3 translation2 = Vec3(quadPos.x * 2 + scale.x / 2, quadPos.y * 2 + scale.y / 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation2) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * 
			Matrix4x4::Translation(translation1) * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_Texture;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = Vec4(1, 1, 0, 0); // Offset & Scaling
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();
		drawItem.textureIndex = textureIndex;
		drawItem.samplerIndex = 0;
		
		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawTexture(RPI::Texture* texture, Vec2 size, 
		bool repeatX, bool repeatY, 
		Vec2 textureScale, Vec2 textureOffset)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		int textureIndex = 0;

		if (textureIndices.KeyExists(texture))
		{
			textureIndex = textureIndices[texture];
		}
		else
		{
			textureIndex = textureCount;
			if (textures.GetSize() < textureCount + 1)
				textures.Resize(textureCount + 1);

			textureIndices[texture] = textureIndex;

			textureCount++;
		}

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];

		textures[textureIndex] = texture;

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation1 = Vec3(-scale.x / 2, -scale.y / 2);
		Vec3 translation2 = Vec3(quadPos.x * 2 + scale.x / 2, quadPos.y * 2 + scale.y / 2, 0);

		RHI::SamplerDescriptor textureSampler{};
		textureSampler.borderColor = SamplerBorderColor::FloatTransparentBlack;
		textureSampler.enableAnisotropy = false;
		textureSampler.samplerFilterMode = FilterMode::Cubic;
		textureSampler.addressModeW = SamplerAddressMode::ClampToBorder;

		if (repeatX)
			textureSampler.addressModeU = SamplerAddressMode::Repeat;
		if (repeatY)
			textureSampler.addressModeV = SamplerAddressMode::Repeat;

		drawItem.transform = Matrix4x4::Translation(translation2) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() *
			Matrix4x4::Translation(translation1) * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_Texture;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = Vec4(textureScale, textureOffset); // Offset & Scaling
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();
		drawItem.textureIndex = textureIndex;
		drawItem.samplerIndex = samplerIndices[textureSampler];
		
		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return size;
	}

	Vec2 Renderer2D::DrawFrameBuffer(const StaticArray<RPI::Texture*, RHI::Limits::MaxSwapChainImageCount>& frames, Vec2 size)
	{
		if (size.x <= 0 || size.y <= 0)
			return Vec2(0, 0);

		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		StaticArray<int, RHI::Limits::MaxSwapChainImageCount> textureIndexPerImage{};

		for (int imageIdx = 0; imageIdx < frames.GetSize(); ++imageIdx)
		{
			RPI::Texture* frame = frames[imageIdx];

			if (textureIndices.KeyExists(frame))
			{
				textureIndexPerImage[imageIdx] = textureIndices[frame];
			}
			else
			{
				textureIndexPerImage[imageIdx] = textureCount;
				if (textures.GetSize() < textureCount + 1)
					textures.Resize(textureCount + 1);

				textureIndices[frame] = textureIndexPerImage[imageIdx];

				textureCount++;
			}

			textures[textureIndexPerImage[imageIdx]] = frame;
		}

		DrawBatch& curDrawBatch = drawBatches.Top();

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		DrawItem2D& drawItem = drawItems[drawItemCount];

		drawItem.transform = Matrix4x4::Translation(translation) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * Matrix4x4::Scale(scale);
		drawItem.drawType = DRAW_FrameBuffer;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = Vec4(1, 1, 0, 0);  // Offset & Scaling
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();
		drawItem.textureIndex = textureIndexPerImage[0]; // Add only the index to first image in frame buffer

		curDrawBatch.drawItemCount++;

		drawItemCount++;

		return size;
	}

	void Renderer2D::End()
	{
		ZoneScoped;

		PopClipRect();

		fontStack.Pop(); // Default font

		const auto& vertBuffers = RPISystem::Get().GetTextQuad();
		RHI::DrawLinearArguments drawArgs = RPISystem::Get().GetTextQuadDrawArgs();

		MaterialHash currentMaterialHash = 0;
		DrawPacket* drawPacket = nullptr;

		Array<DrawPacket*> oldPackets = this->drawPackets;
		this->drawPackets.Clear();

		for (int i = 0; i < drawBatches.GetSize(); i++)
		{
			const DrawBatch& drawBatch = drawBatches[i];

			if (oldPackets.NotEmpty()) // Reuse draw packet
			{
				drawPacket = oldPackets[0];
				oldPackets.RemoveAt(0);

				RPI::Material* material = GetOrCreateMaterial(drawBatch);

				drawPacket->drawItems[0].arguments.linearArgs.firstInstance = drawBatch.firstDrawItemIndex;
				drawPacket->drawItems[0].arguments.linearArgs.instanceCount = drawBatch.drawItemCount;

				drawPacket->shaderResourceGroups[0] = material->GetShaderResourceGroup();
				drawPacket->shaderResourceGroups[1] = drawItemSrg;
				drawPacket->shaderResourceGroups[2] = perViewSrg;

				this->drawPackets.Add(drawPacket);
			}
			else // Create new draw packet
			{
				RHI::DrawPacketBuilder builder{};

				drawArgs.firstInstance = drawBatch.firstDrawItemIndex;
				drawArgs.instanceCount = drawBatch.drawItemCount;
				builder.SetDrawArguments(drawArgs);

				RPI::Material* material = GetOrCreateMaterial(drawBatch);

				builder.AddShaderResourceGroup(material->GetShaderResourceGroup());
				builder.AddShaderResourceGroup(drawItemSrg);
				builder.AddShaderResourceGroup(perViewSrg);

				// UI Item
				{
					RHI::DrawPacketBuilder::DrawItemRequest request{};
					request.vertexBufferViews.AddRange(vertBuffers);

					request.drawItemTag = drawListTag;
					request.drawFilterMask = RHI::DrawFilterMask::ALL;
					RPI::Shader* shader = material->GetShaderCollection()->At(0).shader;
					
					request.pipelineState = shader->GetVariant(0)->GetPipeline(multisampling);

					builder.AddDrawItem(request);
				}

				drawPacket = builder.Build();
				this->drawPackets.Add(drawPacket);
			}
		}

		while (oldPackets.NotEmpty())
		{
			delete oldPackets[0];
			oldPackets.RemoveAt(0);
		}
	}

	const Array<DrawPacket*>& Renderer2D::FlushDrawPackets(u32 imageIndex)
	{
		ZoneScoped;

		curImageIndex = (int)imageIndex;

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

		// - Update View Constants

		if (viewConstantsUpdateRequired[imageIndex])
		{
			viewConstantsUpdateRequired[imageIndex] = false;

			viewConstantsBuffer[imageIndex]->UploadData(&viewConstants, sizeof(viewConstants));
		}

		// - Flush Material Properties -

		for (auto [hash, material] : materials)
		{
			material->FlushProperties(imageIndex);
		}

		// - Update Item Buffer -

		if (drawItemsBuffer[imageIndex] != nullptr && resubmitDrawData[imageIndex])
		{
			resubmitDrawData[imageIndex] = false;

			if (drawItemsBuffer[imageIndex]->GetBufferSize() < drawItemCount * sizeof(DrawItem2D))
			{
				IncrementCharacterDrawItemBuffer(drawItemCount + 10);
			}
			
			void* data;
			drawItemsBuffer[imageIndex]->Map(0, drawItemsBuffer[imageIndex]->GetBufferSize(), &data);
			{
				memcpy(data, drawItems.GetData(), drawItemCount * sizeof(DrawItem2D));
			}
			drawItemsBuffer[imageIndex]->Unmap();
		}

		// - Update Clip Rects Buffer -

		if (clipRectsBuffer[imageIndex] != nullptr && resubmitClipRects[imageIndex])
		{
			resubmitClipRects[imageIndex] = false;

			if (clipRectsBuffer[imageIndex]->GetBufferSize() < clipRectCount * sizeof(ClipRect2D))
			{
				IncrementClipRectsBuffer(clipRectCount + 10);
			}

			void* data;
			clipRectsBuffer[imageIndex]->Map(0, clipRectsBuffer[imageIndex]->GetBufferSize(), &data);
			{
				memcpy(data, clipRects.GetData(), clipRectCount * sizeof(ClipRect2D));
			}
			clipRectsBuffer[imageIndex]->Unmap();
		}

		// - Update Gradient Keys -

		if (gradientKeysBuffer[imageIndex] != nullptr && resubmitGradientKeys[imageIndex])
		{
			resubmitGradientKeys[imageIndex] = false;

			if (gradientKeysBuffer[imageIndex]->GetBufferSize() < gradientKeyCount * sizeof(ColorGradient::Key))
			{
				IncrementGradientKeysBuffer(gradientKeyCount + 10);
			}

			void* data;
			gradientKeysBuffer[imageIndex]->Map(0, gradientKeysBuffer[imageIndex]->GetBufferSize(), &data);
			{
				memcpy(data, gradientKeys.GetData(), gradientKeyCount * sizeof(ColorGradient::Key));
			}
			gradientKeysBuffer[imageIndex]->Unmap();
		}

		// - Update Texture Array -

		Array<RHI::TextureView*> textureViews{};
		textureViews.Reserve(textureCount);

		for (int i = 0; i < textureCount; ++i)
		{
			textureViews.Add(textures[i]->GetOrCreateTextureView());
		}

		if (textureCount > 0)
		{
			drawItemSrg->Bind("_Textures", textureCount, textureViews.GetData());
		}

		drawItemSrg->FlushBindings();
		
		return drawPackets;
	}

	void Renderer2D::SetViewConstants(const PerViewConstants& viewConstants)
	{
		for (int i = 0; i < viewConstantsUpdateRequired.GetSize(); i++)
		{
			viewConstantsUpdateRequired[i] = true;
		}

		this->viewConstants = viewConstants;
	}

	Renderer2D::DrawItem2D& Renderer2D::DrawCustomItem(DrawType drawType, Vec2 size)
	{
		const FontInfo& font = fontStack.Top();

		if constexpr (ForceDisableBatching)
		{
			createNewDrawBatch = true;
		}

		if (drawBatches.IsEmpty() || createNewDrawBatch)
		{
			createNewDrawBatch = false;
			drawBatches.Add({});
			drawBatches.Top().firstDrawItemIndex = drawItemCount;
			drawBatches.Top().font = font;
		}

		if (drawItems.GetSize() < drawItemCount + 1)
			drawItems.Resize(drawItemCount + 1);

		DrawBatch& curDrawBatch = drawBatches.Top();

		DrawItem2D& drawItem = drawItems[drawItemCount];
		drawItem = {};

		Vec3 scale = Vec3(1, 1, 1);

		// Need to multiply by 2 because final range is [-w, w] instead of [0, w]
		scale.x = size.width * 2;
		scale.y = size.height * 2;

		Vec2 quadPos = cursorPosition;
		//Vec3 translation = Vec3(quadPos.x * 2, quadPos.y * 2, 0);

		Vec3 translation1 = Vec3(-scale.x / 2, -scale.y / 2);
		Vec3 translation2 = Vec3(quadPos.x * 2 + scale.x / 2, quadPos.y * 2 + scale.y / 2, 0);

		drawItem.transform = Matrix4x4::Translation(translation2) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() *
			Matrix4x4::Translation(translation1) * Matrix4x4::Scale(scale);

		//drawItem.transform = Matrix4x4::Translation(translation) * Quat::EulerDegrees(Vec3(0, 0, rotation)).ToMatrix() * Matrix4x4::Scale(scale);
		drawItem.drawType = drawType;
		drawItem.fillColor = fillColor.ToVec4();
		drawItem.outlineColor = outlineColor.ToVec4();
		drawItem.itemSize = size;
		drawItem.borderThickness = borderThickness;
		drawItem.bold = 0;
		drawItem.clipRectIdx = clipRectStack.Top();

		curDrawBatch.drawItemCount++;

		drawItemCount++;
		return drawItem;
	}

	void Renderer2D::IncrementCharacterDrawItemBuffer(u32 numCharactersToAdd)
	{
		ZoneScoped;

		u32 curNumItems = drawItemsBuffer[0]->GetBufferSize() / sizeof(DrawItem2D);
		u32 incrementCount = (u32)(curNumItems * 0.25f); // Add 25% to the storage

		numCharactersToAdd = Math::Max(numCharactersToAdd, incrementCount);

		for (int i = 0; i < numFramesInFlight; i++)
		{
			RHI::Buffer* original = drawItemsBuffer[i];

			RHI::BufferDescriptor newBufferDesc{};
			newBufferDesc.name = "DrawItem Buffer";
			newBufferDesc.bindFlags = RHI::BufferBindFlags::StructuredBuffer;
			newBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			newBufferDesc.structureByteStride = sizeof(DrawItem2D);
			newBufferDesc.bufferSize = original->GetBufferSize() + numCharactersToAdd * sizeof(DrawItem2D);

			drawItemsBuffer[i] = RHI::gDynamicRHI->CreateBuffer(newBufferDesc);
			drawItemSrg->Bind(i, "_DrawList", drawItemsBuffer[i]);
			
			void* data;
			original->Map(0, original->GetBufferSize(), &data);
			{
				drawItemsBuffer[i]->UploadData(data, original->GetBufferSize());
			}
			original->Unmap();

			QueueDestroy(original);
		}

		drawItemSrg->FlushBindings();
	}

	void Renderer2D::IncrementClipRectsBuffer(u32 numRectsToAdd)
	{
		ZoneScoped;

		u32 curNumRects = clipRectsBuffer[0]->GetBufferSize() / sizeof(ClipRect2D);
		u32 incrementCount = (u32)(curNumRects * 0.25f); // Add 25% to the storage

		numRectsToAdd = Math::Max(numRectsToAdd, incrementCount);

		for (int i = 0; i < numFramesInFlight; ++i)
		{
			RHI::Buffer* original = clipRectsBuffer[i];

			RHI::BufferDescriptor newBufferDesc{};
			newBufferDesc.name = "ClipRects Buffer";
			newBufferDesc.bindFlags = RHI::BufferBindFlags::StructuredBuffer;
			newBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			newBufferDesc.structureByteStride = sizeof(ClipRect2D);
			newBufferDesc.bufferSize = original->GetBufferSize() + numRectsToAdd * sizeof(ClipRect2D);

			clipRectsBuffer[i] = gDynamicRHI->CreateBuffer(newBufferDesc);
			drawItemSrg->Bind(i, "_ClipRects", clipRectsBuffer[i]);

			void* data;
			original->Map(0, original->GetBufferSize(), &data);
			{
				clipRectsBuffer[i]->UploadData(data, original->GetBufferSize());
			}
			original->Unmap();

			QueueDestroy(original);
		}

		drawItemSrg->FlushBindings();
	}

	void Renderer2D::IncrementGradientKeysBuffer(u32 numKeysToAdd)
	{
		ZoneScoped;

		u32 curNumKeys = gradientKeysBuffer[0]->GetBufferSize() / sizeof(ColorGradient::Key);
		u32 incrementCount = (u32)(curNumKeys * 0.25f); // Add 25% to the storage

		numKeysToAdd = Math::Max(numKeysToAdd, incrementCount);

		for (int i = 0; i < numFramesInFlight; ++i)
		{
			RHI::Buffer* original = gradientKeysBuffer[i];

			RHI::BufferDescriptor newBufferDesc{};
			newBufferDesc.name = "GradientKeys Buffer";
			newBufferDesc.bindFlags = RHI::BufferBindFlags::StructuredBuffer;
			newBufferDesc.defaultHeapType = MemoryHeapType::Upload;
			newBufferDesc.structureByteStride = sizeof(ColorGradient::Key);
			newBufferDesc.bufferSize = original->GetBufferSize() + numKeysToAdd * sizeof(ColorGradient::Key);

			gradientKeysBuffer[i] = gDynamicRHI->CreateBuffer(newBufferDesc);
			drawItemSrg->Bind(i, "_GradientKeys", gradientKeysBuffer[i]);

			void* data;
			original->Map(0, original->GetBufferSize(), &data);
			{
				gradientKeysBuffer[i]->UploadData(data, original->GetBufferSize());
			}
			original->Unmap();

			QueueDestroy(original);
		}

		drawItemSrg->FlushBindings();
	}

	RPI::Material* Renderer2D::GetOrCreateMaterial(const DrawBatch& drawBatch)
	{
		ZoneScoped;

		Name fontName = drawBatch.font.fontName;
		bool isBold = drawBatch.font.bold;
		u32 fontSize = drawBatch.font.fontSize;
		if (fontSize < 4)
			return nullptr;

		RPI::FontAtlasAsset* fontAtlas = fontAtlasesByName[fontName];
		if (fontAtlas == nullptr)
			return nullptr;

		RPI::Material* material = materials[drawBatch.GetMaterialHash()];
		if (material)
			return material;

		material = new RPI::Material(drawShader);
		material->SetPropertyValue("_FontAtlas", fontAtlas->GetAtlasTexture()->GetRpiTexture());
		material->SetPropertyValue("_CharacterData", fontAtlas->GetCharacterLayoutBuffer());
		material->SetPropertyValue("_SDFSmoothness", 50.0f);
		material->FlushProperties();

		materials[drawBatch.GetMaterialHash()] = material;
		return material;
	}

} // namespace CE::RPI
