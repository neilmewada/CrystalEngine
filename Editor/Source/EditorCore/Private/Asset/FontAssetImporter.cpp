#include "EditorCore.h"

namespace CE::Editor
{
	static u64 CalculateTotalTextureSize(u32 width, u32 height, u32 bitsPerPixel, u32 arrayCount = 1, u32 mipLevelCount = 1)
	{
		u64 size = 0;

		for (int mip = 0; mip < mipLevelCount; mip++)
		{
			u32 power = (u32)pow(2, mip);
			u64 curSize = width / power * height / power * bitsPerPixel / 8 * arrayCount;
			size += curSize;
		}

		return size;
	}

	Array<AssetImportJob*> FontAssetImporter::CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets)
	{
		Array<AssetImportJob*> jobs{};

		for (int i = 0; i < sourceAssets.GetSize(); i++)
		{
			FontAssetImportJob* job = new FontAssetImportJob(this, sourceAssets[i], productAssets[i]);
			job->compressFontAtlas = compressFontAtlas;
			job->spread = spread;

			job->genInfo.padding = padding;
			job->genInfo.fontSize = fontSize;

			job->genInfo.charSetRanges.Add(CharRange('a', 'z'));
			job->genInfo.charSetRanges.Add(CharRange('A', 'Z'));
			job->genInfo.charSetRanges.Add(CharRange('0', '9'));
			job->genInfo.charSetRanges.Add(CharRange(32, 47));
			job->genInfo.charSetRanges.Add(CharRange(58, 64));
			job->genInfo.charSetRanges.Add(CharRange(91, 96));
			job->genInfo.charSetRanges.Add(CharRange(123, 126));

			jobs.Add(job);
		}

		return jobs;
	}

	Array<Name> FontAssetImporter::GetProductAssetDependencies()
	{
		static const Array<Name> dependencies{
			"/Editor/Assets/Shaders/UI/SDFTextGen",
			"/Editor/Assets/Shaders/Utils/MipMapGen",
		};

		return dependencies;
	}

	bool FontAssetImportJob::ProcessAsset(const Ref<Bundle>& bundle)
	{
		if (bundle == nullptr)
			return false; // Should never happen
		if (!sourcePath.Exists())
			return false;

		// Clear the bundle of any subobjects/assets, we will build the asset from scratch
		bundle->DestroyAllSubObjects();

		String fileName = sourcePath.GetFileName().RemoveExtension().GetString();
		// Make sure we can use the fileName as name of an object
		fileName = FixObjectName(fileName);

		///////////////////////////////////////////////////
		// - Create rasterized atlas -

		CMFontAtlas* cmFontAtlas = CMFontAtlas::GenerateFromFontFile(sourcePath, genInfo);
		if (cmFontAtlas == nullptr)
		{
			errorMessage = "Failed to parse TTF Font!";
			return false;
		}

		defer(&)
		{
			delete cmFontAtlas;
		};

		const CMImage& fontAtlasImage = cmFontAtlas->GetAtlas();
		if (!fontAtlasImage.IsValid())
		{
			errorMessage = "Invalid font atlas image!";
			return false;
		}
		
		///////////////////////////////////////////////////
		// - Setup objects -

		Ref<Font> font = CreateObject<Font>(bundle.Get(), fileName);
		font->atlasAsset = CreateObject<RPI::FontAtlasAsset>(font.Get(), "AtlasData");
		RPI::FontAtlasAsset* atlasAsset = font->atlasAsset;
		atlasAsset->fontAtlasTexture = CreateObject<TextureAsset>(atlasAsset, "AtlasTexture");

		const auto& glyphInfos = cmFontAtlas->GetGlyphInfos();
		font->atlasAsset->glyphLayouts.Resize(glyphInfos.GetSize());

		atlasAsset->metrics.ascender = cmFontAtlas->GetMetrics().ascender;
		atlasAsset->metrics.descender = cmFontAtlas->GetMetrics().descender;
		atlasAsset->metrics.lineGap = cmFontAtlas->GetMetrics().lineGap;
		atlasAsset->metrics.lineHeight = cmFontAtlas->GetMetrics().lineHeight;
		atlasAsset->metrics.fontSize = genInfo.fontSize;

		for (int i = 0; i < glyphInfos.GetSize(); i++)
		{
			atlasAsset->glyphLayouts[i].unicode = glyphInfos[i].charCode;
			atlasAsset->glyphLayouts[i].x0 = glyphInfos[i].x0;
			atlasAsset->glyphLayouts[i].x1 = glyphInfos[i].x1;
			atlasAsset->glyphLayouts[i].y0 = glyphInfos[i].y0;
			atlasAsset->glyphLayouts[i].y1 = glyphInfos[i].y1;

			atlasAsset->glyphLayouts[i].xOffset = glyphInfos[i].xOffset;
			atlasAsset->glyphLayouts[i].yOffset = glyphInfos[i].yOffset;

			atlasAsset->glyphLayouts[i].advance = glyphInfos[i].advance;
		}

		atlasAsset->fontAtlasTexture->addressModeU = atlasAsset->fontAtlasTexture->addressModeV = atlasAsset->fontAtlasTexture->addressModeW
			= RHI::SamplerAddressMode::ClampToBorder;
		atlasAsset->fontAtlasTexture->borderColor = RHI::SamplerBorderColor::FloatOpaqueBlack;
		atlasAsset->fontAtlasTexture->anisotropy = 0;
		atlasAsset->fontAtlasTexture->arrayLayers = 1;
		atlasAsset->fontAtlasTexture->mipLevels = 1;
		atlasAsset->fontAtlasTexture->filterMode = RHI::FilterMode::Linear;
		atlasAsset->fontAtlasTexture->format = RHI::Format::R8_UNORM;
		atlasAsset->fontAtlasTexture->width = fontAtlasImage.GetWidth();
		atlasAsset->fontAtlasTexture->height = fontAtlasImage.GetHeight();

		////////////////////////////////////////////////////
		// - Create resources -

		int mipLevels = ceil(log2(Math::Max(fontAtlasImage.GetWidth(), fontAtlasImage.GetHeight()))) + 1;
		mipLevels = Math::Min(mipLevels, 3);
		atlasAsset->fontAtlasTexture->mipLevels = mipLevels;

		u64 totalTextureSize = CalculateTotalTextureSize(fontAtlasImage.GetWidth(), fontAtlasImage.GetHeight(), 8, 1, mipLevels);

		RHI::SamplerDescriptor samplerDesc{};
		samplerDesc.addressModeU = samplerDesc.addressModeV = samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
		samplerDesc.borderColor = SamplerBorderColor::FloatOpaqueBlack;
		samplerDesc.enableAnisotropy = false;
		samplerDesc.samplerFilterMode = FilterMode::Linear;

		RHI::Sampler* sampler = RPISystem::Get().FindOrCreateSampler(samplerDesc);

		RHI::Texture* rasterizedAtlas = nullptr;
		RPI::Texture* rasterizedAtlasRpi = nullptr;
		{
			RHI::TextureDescriptor textureDesc{};
			textureDesc.name = "Raster Atlas";
			textureDesc.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
			textureDesc.defaultHeapType = MemoryHeapType::Default;
			textureDesc.width = fontAtlasImage.GetWidth();
			textureDesc.height = fontAtlasImage.GetHeight();
			textureDesc.depth = 1;
			textureDesc.arrayLayers = 1;
			textureDesc.mipLevels = 1;
			textureDesc.format = RHI::Format::R8_UNORM;
			textureDesc.dimension = RHI::Dimension::Dim2D;
			textureDesc.sampleCount = 1;

			rasterizedAtlas = RHI::gDynamicRHI->CreateTexture(textureDesc);
			rasterizedAtlasRpi = new RPI::Texture(rasterizedAtlas, samplerDesc);
		}

		RHI::Texture* sdfFontAtlas = nullptr;
		RPI::Texture* sdfFontAtlasRpi = nullptr;
		{
			RHI::TextureDescriptor desc{};
			desc.name = "Texture";
			desc.arrayLayers = 1;
			desc.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
			desc.width = rasterizedAtlas->GetWidth();
			desc.height = rasterizedAtlas->GetHeight();
			desc.depth = 1;
			desc.defaultHeapType = MemoryHeapType::Default;
			desc.format = RHI::Format::R8_UNORM;
			desc.dimension = Dimension::Dim2D;
			desc.mipLevels = mipLevels;
			desc.sampleCount = 1;

			sdfFontAtlas = RHI::gDynamicRHI->CreateTexture(desc);
			sdfFontAtlasRpi = new RPI::Texture(sdfFontAtlas);
		}

		Array<RHI::TextureView*> sdfFontAtlasMipViews{};

		for (int i = 0; i < mipLevels; i++)
		{
			RHI::TextureViewDescriptor view{};
			view.texture = sdfFontAtlas;
			view.arrayLayerCount = 1;
			view.baseArrayLayer = 0;
			view.baseMipLevel = i;
			view.mipLevelCount = 1;
			view.format = sdfFontAtlas->GetFormat();
			view.dimension = sdfFontAtlas->GetDimension();
			
			sdfFontAtlasMipViews.Add(RHI::gDynamicRHI->CreateTextureView(view));
		}

		defer(&)
		{
			for (int i = 0; i < mipLevels; i++)
			{
				delete sdfFontAtlasMipViews[i];
			}
		};

		// - Intermediate resources -

		RHI::Buffer* inputBuffer = nullptr;
		{
			RHI::BufferDescriptor desc{};
			desc.name = "Input Buffer";
			desc.bindFlags = BufferBindFlags::StagingBuffer;
			desc.defaultHeapType = MemoryHeapType::Upload;
			desc.bufferSize = rasterizedAtlas->GetWidth() * rasterizedAtlas->GetHeight();

			inputBuffer = RHI::gDynamicRHI->CreateBuffer(desc);

			// Upload rasterized image data
			void* inData;
			inputBuffer->Map(0, inputBuffer->GetBufferSize(), &inData);
			{
				memcpy(inData, fontAtlasImage.GetDataPtr(), inputBuffer->GetBufferSize());
			}
			inputBuffer->Unmap();
		}

		RHI::Buffer* outputBuffer = nullptr;
		{
			RHI::BufferDescriptor desc{};
			desc.name = "Output Image";
			desc.bindFlags = BufferBindFlags::StagingBuffer;
			desc.defaultHeapType = MemoryHeapType::ReadBack;
			desc.bufferSize = totalTextureSize;

			outputBuffer = RHI::gDynamicRHI->CreateBuffer(desc);
		}

		defer(&)
		{
			delete rasterizedAtlasRpi;
			delete sdfFontAtlasRpi;
			delete inputBuffer;
			delete outputBuffer;
		};

		/////////////////////////////////////////
		// - Setup shaders & materials -

		Ref<CE::Shader> sdfGenShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/UI/SDFTextGen");
		RPI::Material* sdfGenMaterial = new RPI::Material(sdfGenShader->GetShaderCollection()->At(0).shader);
		sdfGenMaterial->SetPropertyValue("_FontAtlas", rasterizedAtlasRpi);
		sdfGenMaterial->SetPropertyValue("_Spread", spread);
		sdfGenMaterial->FlushProperties();

		defer(&)
		{
			delete sdfGenMaterial;
		};

		Ref<CE::Shader> mipMapShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/Utils/MipMapGen");
		Array<RPI::Material*> mipMapMaterials{};

		for (int i = 1; i < mipLevels; i++)
		{
			RPI::Material* mipMapMaterial = new RPI::Material(mipMapShader->GetShaderCollection()->At(0).shader);
			mipMapMaterial->SetPropertyValue("_InputTexture", sdfFontAtlasMipViews[i - 1]);
			mipMapMaterial->SetPropertyValue("_InputSampler", sampler);
			mipMapMaterial->FlushProperties();
			mipMapMaterials.Add(mipMapMaterial);
		}

		defer(&)
		{
			for (int i = 0; i < mipMapMaterials.GetSize(); i++)
			{
				delete mipMapMaterials[i];
			}
		};

		////////////////////////////////////////
		// - Setup RHI -

		auto queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
		auto cmdList = RHI::gDynamicRHI->AllocateCommandList(queue);
		auto fence = RHI::gDynamicRHI->CreateFence(0);

		const auto& fullScreenQuad = RPISystem::Get().GetFullScreenQuad();
		auto drawArgs = RPISystem::Get().GetFullScreenQuadDrawArgs();

		RHI::RenderPass* renderPass = nullptr;
		Array<RHI::RenderPassFrameBuffer*> sdfFontAtlasMipFrameBuffers{};

		{
			RHI::RenderPassLayout rpLayout{};
			RHI::RenderPassAttachmentLayout colorAttachment{};
			colorAttachment.attachmentId = "Color";
			colorAttachment.attachmentUsage = ScopeAttachmentUsage::Color;
			colorAttachment.format = RHI::Format::R8_UNORM;
			colorAttachment.multisampleState.sampleCount = 1;
			colorAttachment.loadAction = AttachmentLoadAction::Clear;
			colorAttachment.storeAction = AttachmentStoreAction::Store;
			rpLayout.attachmentLayouts.Add(colorAttachment);

			renderPass = RHI::gDynamicRHI->CreateRenderPass(rpLayout);
			//renderTargetBuffer = RHI::gDynamicRHI->CreateRenderTargetBuffer(renderTarget, { sdfFontAtlasMipViews[0] });

			for (int i = 0; i < mipLevels; i++)
			{
				sdfFontAtlasMipFrameBuffers.Add(RHI::gDynamicRHI->CreateRenderPassFrameBuffer({ renderPass, { sdfFontAtlasMipViews[i] } }));
			}
		}

		defer(&)
		{
			delete cmdList;
			delete fence;
			delete renderPass;
			for (int i = 0; i < mipLevels; i++)
			{
				delete sdfFontAtlasMipFrameBuffers[i];
			}
		};

		// - Record Commands -

		cmdList->Begin();
		{
			RHI::ResourceBarrierDescriptor barrier{};
			barrier.subresourceRange = RHI::SubresourceRange::All();

			barrier.resource = inputBuffer;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::CopySource;
			cmdList->ResourceBarrier(1, &barrier);

			barrier.resource = outputBuffer;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::CopyDestination;
			cmdList->ResourceBarrier(1, &barrier);

			barrier.resource = rasterizedAtlas;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::CopyDestination;
			cmdList->ResourceBarrier(1, &barrier);

			// Transfer: Input Buffer -> Rasterized Atlas Texture
			{
				RHI::BufferToTextureCopy copy{};
				copy.srcBuffer = inputBuffer;
				copy.bufferOffset = 0;
				copy.dstTexture = rasterizedAtlas;
				copy.baseArrayLayer = 0;
				copy.layerCount = 1;
				copy.mipSlice = 0;
				
				cmdList->CopyTextureRegion(copy);
			}

			barrier.resource = rasterizedAtlas;
			barrier.fromState = ResourceState::CopyDestination;
			barrier.toState = ResourceState::FragmentShaderResource;
			cmdList->ResourceBarrier(1, &barrier);

			barrier.resource = sdfFontAtlas;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::ColorOutput;
			cmdList->ResourceBarrier(1, &barrier);

			RHI::AttachmentClearValue clear{};
			clear.clearValue = Vec4(0, 0, 0, 0);

			// Render SDF atlas into Mip 0

			cmdList->BeginRenderPass(renderPass, sdfFontAtlasMipFrameBuffers[0], &clear);
			{
				RHI::ViewportState viewport{};
				viewport.x = viewport.y = 0;
				viewport.width = sdfFontAtlas->GetWidth();
				viewport.height = sdfFontAtlas->GetHeight();
				viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
				cmdList->SetViewports(1, &viewport);

				RHI::ScissorState scissor{};
				scissor.x = scissor.y = 0;
				scissor.width = viewport.width;
				scissor.height = viewport.height;
				cmdList->SetScissors(1, &scissor);

				RHI::PipelineState* pipeline = sdfGenMaterial->GetCurrentShader()->GetDefaultPipeline();
				cmdList->BindPipelineState(pipeline);

				cmdList->SetShaderResourceGroup(sdfGenMaterial->GetShaderResourceGroup());
				cmdList->CommitShaderResources();

				cmdList->BindVertexBuffers(0, fullScreenQuad.GetSize(), fullScreenQuad.GetData());

				cmdList->DrawLinear(drawArgs);
			}
			cmdList->EndRenderPass();

			barrier.resource = sdfFontAtlas;
			barrier.fromState = ResourceState::ColorOutput;
			barrier.toState = ResourceState::FragmentShaderResource;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			cmdList->ResourceBarrier(1, &barrier);

			cmdList->ClearShaderResourceGroups();

			// Generate Mips

			for (int i = 1; i < mipLevels; i++)
			{
				cmdList->BeginRenderPass(renderPass, sdfFontAtlasMipFrameBuffers[i], &clear);

				RHI::ViewportState viewport{};
				viewport.x = viewport.y = 0;
				viewport.width = sdfFontAtlas->GetWidth(i);
				viewport.height = sdfFontAtlas->GetHeight(i);
				viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
				cmdList->SetViewports(1, &viewport);

				RHI::ScissorState scissor{};
				scissor.x = scissor.y = 0;
				scissor.width = viewport.width;
				scissor.height = viewport.height;
				cmdList->SetScissors(1, &scissor);

				RHI::PipelineState* pipeline = mipMapMaterials[i - 1]->GetCurrentShader()->GetDefaultPipeline();
				cmdList->BindPipelineState(pipeline);

				cmdList->SetShaderResourceGroup(mipMapMaterials[i - 1]->GetShaderResourceGroup());
				cmdList->CommitShaderResources();

				cmdList->BindVertexBuffers(0, fullScreenQuad.GetSize(), fullScreenQuad.GetData());

				cmdList->DrawLinear(drawArgs);

				cmdList->EndRenderPass();

				barrier.resource = sdfFontAtlas;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange.baseMipLevel = i;
				barrier.subresourceRange.levelCount = 1;
				cmdList->ResourceBarrier(1, &barrier);
			}

			barrier.resource = sdfFontAtlas;
			barrier.fromState = ResourceState::FragmentShaderResource;
			barrier.toState = ResourceState::CopySource;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);


			// Transfer: SDF Atlas -> Output buffer
			u64 copyBufferOffset = 0;

			for (int i = 0; i < mipLevels; i++)
			{
				RHI::TextureToBufferCopy copy{};
				copy.srcTexture = sdfFontAtlas;
				copy.baseArrayLayer = 0;
				copy.layerCount = 1;
				copy.mipSlice = i;
				copy.dstBuffer = outputBuffer;
				copy.bufferOffset = copyBufferOffset;

				cmdList->CopyTextureRegion(copy);

				copyBufferOffset += CalculateTotalTextureSize(sdfFontAtlas->GetWidth(i), sdfFontAtlas->GetHeight(i), 8);
			}

			barrier.resource = outputBuffer;
			barrier.fromState = ResourceState::CopyDestination;
			barrier.toState = ResourceState::General;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			barrier.resource = sdfFontAtlas;
			barrier.fromState = ResourceState::CopySource;
			barrier.toState = ResourceState::FragmentShaderResource;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);
		}
		cmdList->End();

		RHI::CommandQueueSubmission submission{};
		submission.numCommandLists = 1;
		submission.commandLists = &cmdList;
		submission.signalFence = fence;
		submission.signalFenceValue = fence->NextSignalValue();

		queue->Submit(submission);
		
		fence->WaitCPU(submission.signalFenceValue);

		// - Read data -

		// BCn formats are Desktop only
		if (!PlatformMisc::IsDesktopPlatform(targetPlatform))
		{
			compressFontAtlas = false;
		}

		// Force disable compression for now
		compressFontAtlas = false;

		if (compressFontAtlas)
		{
			atlasAsset->fontAtlasTexture->format = RHI::Format::BC4_UNORM;
		}

		void* data;
		outputBuffer->Map(0, outputBuffer->GetBufferSize(), &data);
		{
			CMImage img = CMImage::LoadRawImageFromMemory((u8*)data, sdfFontAtlas->GetWidth(), sdfFontAtlas->GetHeight(), CMImageFormat::R8,
				CMImageSourceFormat::None, 8, 8);
			img.EncodeToPNG(PlatformDirectories::GetLaunchDir() / ("Temp/" + fileName + ".png"));

			if (!compressFontAtlas)
			{
				atlasAsset->fontAtlasTexture->source.LoadData(data, outputBuffer->GetBufferSize());
			}
			else
			{
				CMImageEncoder encoder{};
				u64 requiredSize = encoder.GetCompressedSizeRequirement(img, CMImageSourceFormat::BC4);
				atlasAsset->fontAtlasTexture->source.Reserve(requiredSize);

				bool success = encoder.EncodeToBCn(img, atlasAsset->fontAtlasTexture->source.GetDataPtr(), CMImageSourceFormat::BC4, CMImageEncoder::Quality_Slow);
				if (!success)
				{
					return false;
				}
			}
		}
		outputBuffer->Unmap();

		// - Cleanup -

		return true;
	}

} // namespace CE::Editor
