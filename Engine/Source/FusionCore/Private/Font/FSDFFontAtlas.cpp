#include "FusionCore.h"

#undef DLL_EXPORT
#define FT_PUBLIC_FUNCTION_ATTRIBUTE

#include <ft2build.h>
#include "freetype/freetype.h"

#include "FontConstants.inl"

namespace CE
{
    constexpr int SDFSpread = 9;

    FSDFFontAtlas::FSDFFontAtlas()
    {
        padding = 48;
    }

    void FSDFFontAtlas::Init(const FSDFFontAtlasInitInfo& initInfo)
    {
        ZoneScoped;

        if (atlasImageLayers.NotEmpty())
            return;

		sdfGlyphShader = initInfo.sdfGlyphShader;

        RHI::ShaderResourceGroupLayout perMaterialLayout = sdfGlyphShader->GetDefaultVariant()->GetSrgLayout(SRGType::PerMaterial);

        sdfGenMaterial = new RPI::Material(sdfGlyphShader);

        f32 unitsPerEM = regular->units_per_EM;
        f32 scaleFactor = 1.0f / unitsPerEM; // 1.0f is used as a font size here

        metrics.ascender = regular->ascender * scaleFactor;
        metrics.descender = regular->descender * scaleFactor;
        metrics.lineGap = (regular->height - (regular->ascender - regular->descender)) * scaleFactor;
        metrics.lineHeight = (regular->ascender - regular->descender + metrics.lineGap) * scaleFactor;

        Ptr<FAtlasImage> atlas = new FAtlasImage;
        atlas->ptr = new u8[FontAtlasSize * FontAtlasSize];
        memset(atlas->ptr, 0, FontAtlasSize * FontAtlasSize);
        atlas->atlasSize = FontAtlasSize;

        atlasImageLayers.Add(atlas);

        RHI::RenderTargetLayout rtLayout{};
        rtLayout.attachmentLayouts.Add({
            .attachmentId = "ColorOutput",
            .attachmentUsage = ScopeAttachmentUsage::Color,
            .format = Format::R8_UNORM,
            .multisampleState = { .sampleCount = 1, .quality = 1 },
            .loadAction = AttachmentLoadAction::Clear,
            .storeAction = AttachmentStoreAction::Store,
            .loadActionStencil = AttachmentLoadAction::DontCare,
            .storeActionStencil = AttachmentStoreAction::DontCare
        });

		sdfRenderTarget = RHI::gDynamicRHI->CreateRenderTarget(rtLayout);

        for (int i = 0; i < flushRequiredPerImage.GetSize(); ++i)
        {
			flushRequiredPerImage[i] = true;
        }

        RPI::Shader* fusionShader2 = FusionApplication::Get()->GetFusionShader2();

        const RHI::ShaderResourceGroupLayout& materialSrgLayout = fusionShader2->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerMaterial);
        fontSrg2 = RHI::gDynamicRHI->CreateShaderResourceGroup(materialSrgLayout);

        AddGlyphs(initInfo.characterSet);

        fence = RHI::gDynamicRHI->CreateFence(false);

        UpdateAtlas(false);
    }

    u32 FSDFFontAtlas::GetAtlasSize() const
    {
        return FontAtlasSize;
    }

    void FSDFFontAtlas::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        if (RHI::gDynamicRHI == nullptr)
			return;
        if (IsDefaultInstance())
            return;

		delete atlasTexture; atlasTexture = nullptr;
    	delete sdfGenMaterial; sdfGenMaterial = nullptr;
        delete fontSrg2; fontSrg2 = nullptr;

        RHI::gDynamicRHI->DestroyFence(fence); fence = nullptr;
        RHI::gDynamicRHI->DestroyRenderTarget(sdfRenderTarget); sdfRenderTarget = nullptr;
    }

    void FSDFFontAtlas::UpdateAtlas(bool wait)
    {
        if (workSubmitted && !fence->IsSignalled())
            return;

        workSubmitted = false;

        fence->Reset();

    	atlasUpdateRequired = false;

        Array<CMImage> images;
        images.Reserve(atlasImageLayers.GetSize());

        for (FAtlasImage* atlasImageLayer : atlasImageLayers)
        {
            CMImage image = CMImage::LoadRawImageFromMemory(atlasImageLayer->ptr, atlasImageLayer->atlasSize, atlasImageLayer->atlasSize,
                CMImageFormat::R8, CMImageSourceFormat::None, 8, 8);
            images.Add(image);
        }

        sourceImage = new RPI::Texture("SDF Source Atlas", images, SamplerDescriptor{
                .addressModeU = SamplerAddressMode::ClampToBorder,
                .addressModeV = SamplerAddressMode::ClampToBorder,
                .addressModeW = SamplerAddressMode::ClampToBorder,
                .samplerFilterMode = FilterMode::Linear,
                .borderColor = SamplerBorderColor::FloatTransparentBlack,
                .enableAnisotropy = false, .maxAnisotropy = 0 });

        RHI::BufferDescriptor stagingBufferDesc{};
        stagingBufferDesc.bindFlags = BufferBindFlags::StagingBuffer;
        stagingBufferDesc.bufferSize = RHI::Texture::CalculateTotalTextureSize(atlasImageLayers[0]->atlasSize, atlasImageLayers[0]->atlasSize, 8);
        stagingBufferDesc.defaultHeapType = MemoryHeapType::ReadBack;
        stagingBufferDesc.structureByteStride = stagingBufferDesc.bufferSize;

        RPI::TextureDescriptor outputDesc{};
        outputDesc.texture.arrayLayers = atlasImageLayers.GetSize();
        outputDesc.texture.mipLevels = 1;
        outputDesc.texture.width = atlasImageLayers[0]->atlasSize;
        outputDesc.texture.height = atlasImageLayers[0]->atlasSize;
        outputDesc.texture.depth = 1;
        outputDesc.texture.format = Format::R8_UNORM;
        outputDesc.texture.defaultHeapType = MemoryHeapType::Default;
        outputDesc.texture.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
        outputDesc.texture.dimension = Dimension::Dim2D;
        outputDesc.texture.sampleCount = 1;
        outputDesc.texture.name = "SDF Font Atlas";

        outputDesc.samplerDesc.addressModeU = outputDesc.samplerDesc.addressModeV = outputDesc.samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
        outputDesc.samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;
        outputDesc.samplerDesc.enableAnisotropy = false;
        outputDesc.samplerDesc.samplerFilterMode = FilterMode::Linear;

        oldAtlasTexture = atlasTexture;

        atlasTexture = new RPI::Texture(outputDesc);

        fontSrg2->Bind("_FontAtlas", atlasTexture->GetRhiTexture());
        fontSrg2->Bind("_FontAtlasSampler", atlasTexture->GetSamplerState());
        fontSrg2->FlushBindings();

        sdfGenMaterial->SetPropertyValue("_FontAtlas", sourceImage);
        sdfGenMaterial->SetPropertyValue("_Spread", SDFSpread);
        sdfGenMaterial->FlushProperties();

        frameBuffer = RHI::gDynamicRHI->CreateRenderTargetBuffer(sdfRenderTarget, { atlasTexture->GetRhiTexture() });

        RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        cmdList = RHI::gDynamicRHI->AllocateCommandList(queue);

        const auto& fullScreenQuad = RPISystem::Get().GetFullScreenQuad();
        DrawLinearArguments fullScreenQuadArgs = RPISystem::Get().GetFullScreenQuadDrawArgs();

        cmdList->Begin();
        {
            RHI::ResourceBarrierDescriptor barrier{};
            barrier.subresourceRange = RHI::SubresourceRange::All();

            barrier.resource = atlasTexture->GetRhiTexture();
            barrier.fromState = ResourceState::Undefined;
            barrier.toState = ResourceState::ColorOutput;
            cmdList->ResourceBarrier(1, &barrier);

            AttachmentClearValue clear{};
            clear.clearValue = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
            cmdList->BeginRenderTarget(sdfRenderTarget, frameBuffer, &clear);
            {
                RHI::ViewportState viewport{};
                viewport.x = viewport.y = 0;
                viewport.width = atlasTexture->GetWidth();
                viewport.height = atlasTexture->GetHeight();
                viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
                cmdList->SetViewports(1, &viewport);

                RHI::ScissorState scissor{};
                scissor.x = scissor.y = 0;
                scissor.width = viewport.width;
                scissor.height = viewport.height;
                cmdList->SetScissors(1, &scissor);

                GraphicsPipelineVariant pipelineVariant{
                    .sampleState = {.sampleCount = 1, .quality = 1 },
                    .colorFormats = { RHI::Format::R8_UNORM },
                    .depthStencilFormat = RHI::Format::Undefined,
                };

                RHI::PipelineState* pipeline = sdfGenMaterial->GetCurrentShader()->GetDefaultVariant()->GetPipeline(pipelineVariant);

                cmdList->BindPipelineState(pipeline);

                cmdList->SetShaderResourceGroup(sdfGenMaterial->GetShaderResourceGroup());
                cmdList->CommitShaderResources();

                cmdList->BindVertexBuffers(0, fullScreenQuad.GetSize(), fullScreenQuad.GetData());

                cmdList->DrawLinear(fullScreenQuadArgs);
            }
            cmdList->EndRenderTarget();

            barrier.resource = sourceImage->GetRhiTexture();
            barrier.fromState = ResourceState::FragmentShaderResource;
            barrier.toState = ResourceState::CopySource;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = atlasTexture->GetRhiTexture();
            barrier.fromState = ResourceState::ColorOutput;
            barrier.toState = ResourceState::CopySource;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = sourceImage->GetRhiTexture();
            barrier.fromState = ResourceState::CopySource;
            barrier.toState = ResourceState::FragmentShaderResource;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = atlasTexture->GetRhiTexture();
            barrier.fromState = ResourceState::CopySource;
            barrier.toState = ResourceState::FragmentShaderResource;
            cmdList->ResourceBarrier(1, &barrier);
        }
        cmdList->End();

        workSubmitted = true;
        queue->Execute(1, &cmdList, fence);

        if (wait)
        {
            fence->WaitForFence();

            workSubmitted = false;
        }
    }

    void FSDFFontAtlas::Flush(u32 imageIndex)
    {
        ZoneScoped;

        if (workSubmitted && fence->IsSignalled())
        {
            delete oldAtlasTexture; oldAtlasTexture = nullptr;
            delete frameBuffer; frameBuffer = nullptr;
            delete sourceImage; sourceImage = nullptr;

            fontSrg2->Bind("_FontAtlas", atlasTexture->GetRhiTexture());
            fontSrg2->Bind("_FontAtlasSampler", atlasTexture->GetSamplerState());
            fontSrg2->FlushBindings();

            RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);
            cmdList = nullptr;

            fence->Reset();
        }
        else if (workSubmitted)
        {
            return; // We wait for previous work to finish, i.e. when fence is signalled.
        }

        if (!flushRequiredPerImage[imageIndex])
            return;

        if (atlasUpdateRequired)
        {
            atlasUpdateRequired = false;

            UpdateAtlas(false);
        }

		flushRequiredPerImage[imageIndex] = false;
    }

    FFontGlyphInfo FSDFFontAtlas::FindOrAddGlyph(u32 charCode, u32 fontSize, bool isBold, bool isItalic)
    {
        ZoneScoped;
        char __text[2] = { (char)charCode, 0 };
        ZoneText(__text, 1);

        if (!arrayLayerByCharCode.KeyExists(charCode))
        {
            static Array<u32> charSet{};
            charSet.Resize(1);
            charSet[0] = charCode;
            AddGlyphs(charSet, isBold, isItalic);
        }

        if (!arrayLayerByCharCode.KeyExists(charCode))
        {
            return {};
        }

        int layerIndex = arrayLayerByCharCode[charCode];
        Ptr<FAtlasImage> atlasMip = atlasImageLayers[layerIndex];

        if (!atlasMip->glyphsByCharCode.KeyExists(charCode))
        {
            static Array<u32> charSet{};
            charSet.Resize(1);
            charSet[0] = charCode;
            AddGlyphs(charSet, isBold, isItalic);
        }

        if (!atlasMip->glyphsByCharCode.KeyExists(charCode))
        {
            return {};
        }

        return atlasMip->glyphsByCharCode[charCode];
    }

    void FSDFFontAtlas::AddGlyphs(const Array<u32>& charSet, bool isBold, bool isItalic)
    {
        ZoneScoped;

        if (charSet.IsEmpty())
            return;

        FT_Face face = regular;

        if (isBold && isItalic)
        {
            if (boldItalic != nullptr)
                face = boldItalic;
            else if (bold != nullptr)
                face = bold;
        }
        else if (isBold)
        {
            if (bold != nullptr)
                face = bold;
        }
        else if (isItalic)
        {
            if (italic != nullptr)
                face = italic;
        }

        static HashSet<FT_ULong> nonDisplayCharacters = { ' ', '\n', '\r', '\t' };

        u32 dpi = PlatformApplication::Get()->GetSystemDpi();
        f32 scaling = PlatformApplication::Get()->GetSystemDpiScaling();

        FT_Set_Char_Size(face, 0, SDFFontSize << 6, dpi, dpi);
        
        FAtlasImage* atlasMip = atlasImageLayers[currentLayer];

        for (int i = 0; i < charSet.GetSize(); ++i)
        {
            FT_ULong charCode = charSet[i];
            char c = charCode;

            if (atlasMip->glyphsByCharCode.KeyExists(charCode))
            {
                continue;
            }

            FT_Error error = FT_Load_Char(face, charCode, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL);
            if (error != 0)
            {
                continue;
            }

            FT_Bitmap* bmp = &face->glyph->bitmap;

            FFontGlyphInfo glyph{};

            glyph.charCode = charCode;

            int width = bmp->width;
            int height = bmp->rows;

            int xOffset = face->glyph->bitmap_left;
            int yOffset = face->glyph->bitmap_top;
            int advance = face->glyph->advance.x >> 6;

            int posX, posY;
            int atlasSize = atlasMip->atlasSize;

            constexpr int glyphPadding = 4;

            bool foundEmptySpot = atlasMip->TryInsertGlyphRect(Vec2i(width + glyphPadding, height + glyphPadding), padding, posX, posY);
            if (!foundEmptySpot)
            {
                currentLayer++;
                atlasSize = FontAtlasSize / (SIZE_T)Math::Pow(2, currentLayer);

                atlasMip = new FAtlasImage;
                atlasMip->ptr = new u8[atlasSize * atlasSize];
                memset(atlasMip->ptr, 0, atlasSize * atlasSize);
                atlasMip->atlasSize = (u32)atlasSize;

                atlasImageLayers.Add(atlasMip);

                foundEmptySpot = atlasMip->TryInsertGlyphRect(Vec2i(width + glyphPadding, height + glyphPadding), padding, posX, posY);
            }

            if (!foundEmptySpot)
            {
                return;
            }

            posX += glyphPadding / 2;
            posY += glyphPadding / 2;

            arrayLayerByCharCode[charCode] = currentLayer;

            glyph.atlasSize = atlasSize;

            glyph.x0 = posX - glyphPadding / 2;
            glyph.y0 = posY - glyphPadding / 2;
            glyph.x1 = posX + width + glyphPadding / 2;
            glyph.y1 = posY + height + glyphPadding / 2;

            glyph.xOffset = xOffset;
            glyph.yOffset = yOffset;
            glyph.advance = advance;
            glyph.fontSize = SDFFontSize;
            // TODO: index
            glyph.index = 0;//glyphDataList.GetCount();

            glyph.scalingFactor = 1.0f;

            Vec2 uvMin = Vec2((f32)glyph.x0 / (f32)atlasSize, (f32)glyph.y0 / (f32)atlasSize);
            Vec2 uvMax = Vec2((f32)glyph.x1 / (f32)atlasSize, (f32)glyph.y1 / (f32)atlasSize);

            if (!nonDisplayCharacters.Exists(charCode))
            {
                atlasUpdateRequired = true;

                for (int row = 0; row < bmp->rows; ++row)
                {
                    for (int col = 0; col < bmp->width; ++col)
                    {
                        int x = posX + col;
                        int y = posY + row;
                        atlasMip->ptr[y * atlasSize + x] = bmp->buffer[row * bmp->pitch + col];
                    }
                }
            }

			atlasMip->glyphsByCharCode[charCode] = glyph;

            for (int j = 0; j < flushRequiredPerImage.GetSize(); ++j)
            {
                flushRequiredPerImage[j] = true;
            }
        }
    }

    bool FSDFFontAtlas::FAtlasImage::TryInsertGlyphRect(Vec2i glyphSize, int padding, int& outX, int& outY)
    {
        ZoneScoped;

        int bestRowIndex = -1;
        int bestRowHeight = INT_MAX;

        if (rows.IsEmpty())
        {
            rows.Add({ .x = glyphSize.width + padding / 2, .y = padding / 2, .height = glyphSize.height + padding / 2 });
            outX = padding / 2;
            outY = padding / 2;
            return true;
        }

        for (int i = 0; i < rows.GetSize(); ++i)
        {
            int x = rows[i].x;
            int y = rows[i].y;

            // Check if the glyph fits at this position
            if (x + glyphSize.width + padding / 2 <= atlasSize && y + glyphSize.height + padding / 2 <= atlasSize)
            {
                if (rows[i].height >= glyphSize.height + padding / 2 && rows[i].height + padding / 2 < bestRowHeight)
                {
                    bestRowHeight = rows[i].height + padding / 2;
                    bestRowIndex = i;
                }
            }
        }

        if (bestRowIndex == -1)
        {
            RowSegment lastRow = rows.Top();
            if (lastRow.y + lastRow.height + glyphSize.height + padding / 2 > atlasSize)
            {
                return false;
            }

            rows.Add({ .x = glyphSize.width + padding / 2, .y = lastRow.y + lastRow.height + padding / 2, .height = glyphSize.height + padding / 2 });

            outX = padding / 2;
            outY = rows.Top().y;

            return true;
        }

        outX = rows[bestRowIndex].x + padding / 2;
        outY = rows[bestRowIndex].y;

        rows[bestRowIndex].x += glyphSize.width + padding / 2;

        return true;
    }
} // namespace CE

