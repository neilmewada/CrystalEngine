#include "CoreRPI.h"

namespace CE::RPI
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

	bool CubeMapProcessor::ProcessCubeMapOffline(const CubeMapOfflineProcessInfo& desc, BinaryBlob& output, u32& outMipLevels)
	{
#if !PLATFORM_DESKTOP
		return false; // Only for desktop platforms
#endif

		if (!desc.sourceImage.IsValid())
			return false;
		if (desc.sourceImage.GetWidth() != desc.sourceImage.GetHeight() * 2)
			return false;

		CMImageFormat sourceFormat = desc.sourceImage.GetFormat();
		bool isSourceFloatFormat = false;
		if (sourceFormat == CMImageFormat::RGBA32 || sourceFormat == CMImageFormat::RGB32)
		{
			isSourceFloatFormat = true;
		}
		else if (sourceFormat == CMImageFormat::RGBA8 || sourceFormat == CMImageFormat::RGB8)
		{
			isSourceFloatFormat = false;
		}
		else
		{
			return false;
		}

		u32 cubeMapRes = desc.cubeMapResolution;
		if (cubeMapRes == 0)
			cubeMapRes = desc.sourceImage.GetHeight();

		RHI::Format hdriFormat = RHI::Format::R16G16B16A16_SFLOAT;
		bool computeSpecularConvolution = desc.specularConvolution;
		if (desc.specularConvolutionShader == nullptr)
		{
			computeSpecularConvolution = false;
		}

		/////////////////////////////////////////////
		// - Samplers -

		RHI::SamplerDescriptor linearSamplerDesc{};
		linearSamplerDesc.samplerFilterMode = RHI::FilterMode::Linear;
		linearSamplerDesc.enableAnisotropy = false;
		RHI::Sampler* linearSampler = RPISystem::Get().FindOrCreateSampler(linearSamplerDesc);

		linearSamplerDesc.addressModeU = linearSamplerDesc.addressModeV = linearSamplerDesc.addressModeW = SamplerAddressMode::ClampToEdge;
		RHI::Sampler* linearClampedSampler = RPISystem::Get().FindOrCreateSampler(linearSamplerDesc);

		RHI::SamplerDescriptor nearestSamplerDesc{};
		nearestSamplerDesc.samplerFilterMode = RHI::FilterMode::Nearest;
		nearestSamplerDesc.enableAnisotropy = false;
		RHI::Sampler* nearest = RPISystem::Get().FindOrCreateSampler(nearestSamplerDesc);

		nearestSamplerDesc.addressModeU = nearestSamplerDesc.addressModeV = nearestSamplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
		nearestSamplerDesc.borderColor = RHI::SamplerBorderColor::FloatOpaqueBlack;
		nearestSamplerDesc.samplerFilterMode = RHI::FilterMode::Nearest;
		RHI::Sampler* nearestClamped = RPISystem::Get().FindOrCreateSampler(nearestSamplerDesc);

		/////////////////////////////////////////////
		// - Input Resources -

		int totalHdriInputMipLevels = ceil(log2(Math::Max(desc.sourceImage.GetWidth(), desc.sourceImage.GetHeight()))) + 1;
		
		RHI::TextureDescriptor hdriMapDesc{};
		hdriMapDesc.name = "HDRI FlatMap";
		hdriMapDesc.bindFlags = RHI::TextureBindFlags::Color | RHI::TextureBindFlags::ShaderRead;
		hdriMapDesc.width = desc.sourceImage.GetWidth();
		hdriMapDesc.height = desc.sourceImage.GetHeight();
		hdriMapDesc.depth = 1;
		hdriMapDesc.dimension = RHI::Dimension::Dim2D;
		hdriMapDesc.mipLevels = totalHdriInputMipLevels;
		hdriMapDesc.sampleCount = 1;
		hdriMapDesc.arrayLayers = 1;
		hdriMapDesc.format = hdriFormat;

		RHI::Texture* hdriMap = RHI::gDynamicRHI->CreateTexture(hdriMapDesc);
		RPI::Texture* hdriMapRpi = new RPI::Texture(hdriMap);

		/////////////////////////////////////////////
		// - Output Resources -

		int specularCubeMapMipLevels = 1;
		if (computeSpecularConvolution)
		{
			specularCubeMapMipLevels = ceil(log2(cubeMapRes)) + 1;
			specularCubeMapMipLevels = Math::Min(specularCubeMapMipLevels, 10); // 10 mip levels max
			if (specularCubeMapMipLevels > totalHdriInputMipLevels)
				specularCubeMapMipLevels = totalHdriInputMipLevels;

			for (int mip = 0; desc.useCompression && mip < specularCubeMapMipLevels; mip++)
			{
				u32 currentRes = cubeMapRes / (u32)pow(2, mip);
				if (currentRes < 4)
				{
					specularCubeMapMipLevels = mip;
					break;
				}
			}
		}

		outMipLevels = specularCubeMapMipLevels;

		RHI::TextureDescriptor cubeMapDesc{};
		cubeMapDesc.name = desc.name;
		cubeMapDesc.bindFlags = RHI::TextureBindFlags::Color | RHI::TextureBindFlags::ShaderRead;
		cubeMapDesc.defaultHeapType = MemoryHeapType::Default;
		cubeMapDesc.width = cubeMapDesc.height = cubeMapRes;
		cubeMapDesc.depth = 1;
		cubeMapDesc.dimension = RHI::Dimension::DimCUBE;
		cubeMapDesc.mipLevels = specularCubeMapMipLevels;
		cubeMapDesc.sampleCount = 1;
		cubeMapDesc.arrayLayers = 6;
		cubeMapDesc.format = hdriFormat;
		
		// Final CubeMap texture with 6 faces
		RHI::Texture* cubeMap = RHI::gDynamicRHI->CreateTexture(cubeMapDesc);
		RPI::Texture* cubeMapRpi = new RPI::Texture(cubeMap);

		RHI::Texture* diffuseIrradiance = nullptr;
		RPI::Texture* diffuseIrradianceRpi = nullptr;
		
		RHI::Texture* diffuseIrradianceCubeMap = nullptr;
		u32 diffuseIrradianceResolution = desc.diffuseIrradianceResolution;
		if (desc.diffuseConvolutionShader == nullptr)
			diffuseIrradianceResolution = 0;

		if (diffuseIrradianceResolution > 0) // Diffuse irradiance map
		{
			RHI::TextureDescriptor irradianceDesc{};
			irradianceDesc.name = "Diffuse Irradiance Image";
			irradianceDesc.bindFlags = RHI::TextureBindFlags::Color | RHI::TextureBindFlags::ShaderRead;
			irradianceDesc.width = cubeMapRes * 2;
			irradianceDesc.height = cubeMapRes;
			irradianceDesc.depth = 1;
			irradianceDesc.dimension = RHI::Dimension::Dim2D;
			irradianceDesc.mipLevels = 1;
			irradianceDesc.sampleCount = 1;
			irradianceDesc.arrayLayers = 1;
			irradianceDesc.format = hdriFormat;

			diffuseIrradiance = RHI::gDynamicRHI->CreateTexture(irradianceDesc);
			diffuseIrradianceRpi = new RPI::Texture(diffuseIrradiance);
			
			irradianceDesc.dimension = RHI::Dimension::DimCUBE;
			irradianceDesc.width = diffuseIrradianceResolution;
			irradianceDesc.height = diffuseIrradianceResolution;
			irradianceDesc.arrayLayers = 6;

			diffuseIrradianceCubeMap = RHI::gDynamicRHI->CreateTexture(irradianceDesc);
		}

		// CubeMap output buffer
		RHI::BufferDescriptor outputBufferDesc{};
		outputBufferDesc.name = "Output";
		outputBufferDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
		outputBufferDesc.bufferSize = CalculateTotalTextureSize(cubeMapRes, cubeMapRes, GetBitsPerPixelForFormat(cubeMapDesc.format), 6, cubeMapDesc.mipLevels);
		//cubeMapRes * cubeMapRes * GetBitsPerPixelForFormat(cubeMapDesc.format) / 8 * 6;
		outputBufferDesc.defaultHeapType = MemoryHeapType::Upload;

		RHI::Buffer* outputBuffer = RHI::gDynamicRHI->CreateBuffer(outputBufferDesc);

		RHI::BufferDescriptor diffuseIrradianceOutputBufferDesc{};
		diffuseIrradianceOutputBufferDesc.name = "Diffuse Output";
		diffuseIrradianceOutputBufferDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
		diffuseIrradianceOutputBufferDesc.bufferSize = diffuseIrradianceResolution * diffuseIrradianceResolution * GetBitsPerPixelForFormat(hdriFormat) / 8 * 6;
		diffuseIrradianceOutputBufferDesc.defaultHeapType = MemoryHeapType::Upload;

		RHI::Buffer* diffuseIrradianceOutputBuffer = RHI::gDynamicRHI->CreateBuffer(diffuseIrradianceOutputBufferDesc);

		/////////////////////////////////////////////
		// - Intermediate Resources -

		// Full screen quad
		Array<RHI::VertexBufferView> fullscreenQuad = RPISystem::Get().GetFullScreenQuad();
		RHI::DrawLinearArguments fullscreenQuadArgs = RPISystem::Get().GetFullScreenQuadDrawArgs();

		// Cube Model
		RPI::ModelLod* cubeModel = RPI::ModelLod::CreateCubeModel();
		RPI::Mesh* cubeMesh = cubeModel->GetMesh(0);
		const auto& cubeVertInfo = cubeMesh->vertexBufferInfos[0];
		auto cubeVertexBufferView = RHI::VertexBufferView(cubeModel->GetBuffer(cubeVertInfo.bufferIndex), cubeVertInfo.byteOffset, cubeVertInfo.byteCount, cubeVertInfo.stride);

		// Staging buffer
		RHI::BufferDescriptor stagingBufferDesc{};
		stagingBufferDesc.name = "Staging Buffer";
		stagingBufferDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
		stagingBufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
		stagingBufferDesc.bufferSize = hdriMap->GetByteSize();

		// To upload input HDRI equirect image to a texture
		RHI::Buffer* stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingBufferDesc);

		void* dataPtr;
		stagingBuffer->Map(0, stagingBuffer->GetBufferSize(), &dataPtr);
		{
			void* srcPtr = desc.sourceImage.GetDataPtr();
			int numPixels = hdriMapDesc.width * hdriMapDesc.height;

			for (int i = 0; i < numPixels; i++)
			{
				u16* u16Data = (u16*)dataPtr;
				f32* f32Data = (f32*)dataPtr;

				f32 r = *((f32*)srcPtr + 4 * i + 0);
				f32 g = *((f32*)srcPtr + 4 * i + 1);
				f32 b = *((f32*)srcPtr + 4 * i + 2);

				// 16 bit float cannot store values larger than 65,000
				if (r > 65000)
					r = 65000;
				if (g > 65000)
					g = 65000;
				if (b > 65000)
					b = 65000;

				*(u16Data + 4 * i + 0) = Math::ToFloat16(r);
				*(u16Data + 4 * i + 1) = Math::ToFloat16(g);
				*(u16Data + 4 * i + 2) = Math::ToFloat16(b);
				*(u16Data + 4 * i + 3) = Math::ToFloat16(1.0f);
			}
		}
		stagingBuffer->Unmap();
		dataPtr = nullptr;

		RHI::Texture* grayscale = nullptr; RPI::Texture* grayscaleRpi = nullptr;
		RHI::Texture* rowAverage = nullptr; RPI::Texture* rowAverageRpi = nullptr;
		RHI::Texture* columnAverage = nullptr; RPI::Texture* columnAverageRpi = nullptr;
		RHI::Texture* pdfJoint = nullptr; RPI::Texture* pdfJointRpi = nullptr;
		RHI::Texture* pdfMarginal = nullptr; RPI::Texture* pdfMarginalRpi = nullptr;
		RHI::Texture* pdfConditional = nullptr; RPI::Texture* pdfConditionalRpi = nullptr;
		RHI::Texture* cdfMarginalInverse = nullptr; RPI::Texture* cdfMarginalInverseRpi = nullptr;
		RHI::Texture* cdfConditionalInverse = nullptr; RPI::Texture* cdfConditionalInverseRpi = nullptr;

		if (diffuseIrradiance != nullptr)
		{
			auto fillIntermediateDescriptor = [=](RHI::TextureDescriptor& textureDesc)
				{
					textureDesc.bindFlags = RHI::TextureBindFlags::Color | RHI::TextureBindFlags::ShaderRead;
					textureDesc.depth = 1;
					textureDesc.dimension = RHI::Dimension::Dim2D;
					textureDesc.mipLevels = 1;
					textureDesc.sampleCount = 1;
					textureDesc.arrayLayers = 1;
					textureDesc.format = RHI::Format::R16_SFLOAT;
				};

			const u32 singleLineWidth = 40;

			RHI::TextureDescriptor grayscaleDesc{};
			grayscaleDesc.name = "Grayscale";
			grayscaleDesc.width = hdriMapDesc.width;
			grayscaleDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(grayscaleDesc);

			RHI::TextureDescriptor rowAverageDesc{};
			rowAverageDesc.name = "Row Average";
			rowAverageDesc.width = singleLineWidth;
			rowAverageDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(rowAverageDesc);

			RHI::TextureDescriptor columnAverageDesc{};
			columnAverageDesc.name = "Column Average";
			columnAverageDesc.width = 1;
			columnAverageDesc.height = 1;
			fillIntermediateDescriptor(columnAverageDesc);

			RHI::TextureDescriptor pdfJointDesc{};
			pdfJointDesc.name = "PDF Joint";
			pdfJointDesc.width = hdriMapDesc.width;
			pdfJointDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(pdfJointDesc);

			RHI::TextureDescriptor pdfMarginalDesc{};
			pdfMarginalDesc.name = "PDF Marginal";
			pdfMarginalDesc.width = singleLineWidth;
			pdfMarginalDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(pdfMarginalDesc);

			RHI::TextureDescriptor cdfMarginalInverseDesc{};
			cdfMarginalInverseDesc.name = "CDF Marginal Inverse";
			cdfMarginalInverseDesc.width = singleLineWidth;
			cdfMarginalInverseDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(cdfMarginalInverseDesc);

			RHI::TextureDescriptor pdfConditionalDesc{};
			pdfConditionalDesc.name = "PDF Conditional";
			pdfConditionalDesc.width = hdriMapDesc.width;
			pdfConditionalDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(pdfConditionalDesc);

			RHI::TextureDescriptor cdfConditionalInverseDesc{};
			cdfConditionalInverseDesc.name = "CDF Conditional Inverse";
			cdfConditionalInverseDesc.width = hdriMapDesc.width;
			cdfConditionalInverseDesc.height = hdriMapDesc.height;
			fillIntermediateDescriptor(cdfConditionalInverseDesc);

			// TODO: Try allocating all textures' memory in one go
			//ResourceMemoryRequirements requirements[8] = {};
			//u64 addressOffsets[6] = {};
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(grayscaleDesc, requirements[0]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(rowAverageDesc, requirements[1]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(columnAverageDesc, requirements[2]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(pdfJointDesc, requirements[3]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(pdfMarginalDesc, requirements[4]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(cdfMarginalInverseDesc, requirements[5]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(pdfConditionalDesc, requirements[6]);
			//RHI::gDynamicRHI->GetTextureMemoryRequirements(cdfConditionalInverseDesc, requirements[7]);

			grayscale = RHI::gDynamicRHI->CreateTexture(grayscaleDesc);
			rowAverage = RHI::gDynamicRHI->CreateTexture(rowAverageDesc);
			columnAverage = RHI::gDynamicRHI->CreateTexture(columnAverageDesc);
			pdfJoint = RHI::gDynamicRHI->CreateTexture(pdfJointDesc);
			pdfMarginal = RHI::gDynamicRHI->CreateTexture(pdfMarginalDesc);
			cdfMarginalInverse = RHI::gDynamicRHI->CreateTexture(cdfMarginalInverseDesc);
			pdfConditional = RHI::gDynamicRHI->CreateTexture(pdfConditionalDesc);
			cdfConditionalInverse = RHI::gDynamicRHI->CreateTexture(cdfConditionalInverseDesc);

			grayscaleRpi = new RPI::Texture(grayscale);
			rowAverageRpi = new RPI::Texture(rowAverage);
			columnAverageRpi = new RPI::Texture(columnAverage);
			pdfJointRpi = new RPI::Texture(pdfJoint);
			pdfMarginalRpi = new RPI::Texture(pdfMarginal);
			cdfMarginalInverseRpi = new RPI::Texture(cdfMarginalInverse);
			pdfConditionalRpi = new RPI::Texture(pdfConditional);
			cdfConditionalInverseRpi = new RPI::Texture(cdfConditionalInverse);
		}

		RHI::Texture* specularConvolution = nullptr; RPI::Texture* specularConvolutionRpi = nullptr;
		Array<RPI::Texture*> specularConvolutionMipMaps{};

		if (computeSpecularConvolution)
		{
			RHI::TextureDescriptor specularConvolutionDesc{};
			specularConvolutionDesc.name = "Specular Convolution";
			specularConvolutionDesc.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
			specularConvolutionDesc.width = hdriMap->GetWidth();
			specularConvolutionDesc.height = hdriMap->GetHeight();
			specularConvolutionDesc.arrayLayers = 1;
			specularConvolutionDesc.mipLevels = specularCubeMapMipLevels;
			specularConvolutionDesc.depth = 1;
			specularConvolutionDesc.sampleCount = 1;
			specularConvolutionDesc.format = hdriFormat;
			specularConvolutionDesc.dimension = Dimension::Dim2D;
			
			specularConvolution = RHI::gDynamicRHI->CreateTexture(specularConvolutionDesc);
			specularConvolutionRpi = new RPI::Texture(specularConvolution);

			RHI::TextureViewDescriptor viewDesc{};
			viewDesc.texture = specularConvolution;
			viewDesc.format = specularConvolution->GetFormat();
			viewDesc.baseArrayLayer = 0;
			viewDesc.arrayLayerCount = 1;
			viewDesc.dimension = Dimension::Dim2D;
			viewDesc.mipLevelCount = 1;

			for (int mip = 0; mip < specularConvolutionDesc.mipLevels; mip++)
			{
				viewDesc.baseMipLevel = mip;
				
				RHI::TextureView* textureView = RHI::gDynamicRHI->CreateTextureView(viewDesc);
				specularConvolutionMipMaps.Add(new RPI::Texture(textureView));
			}
		}

		/////////////////////////////////////////////
		// - Render Targets & RTBs -

		RHI::RenderPass* hdrRenderPass = nullptr;
		RHI::TextureView* cubeMapRTVs[6] = {};
		RHI::RenderPassFrameBuffer* cubeMapRTBs[6] = {};
		RHI::TextureView* irradianceCubeMapRTVs[6] = {};
		RHI::RenderPassFrameBuffer* irradianceCubeMapRTBs[6] = {};

		// Input HDRI MipMap RTBs
		Array<RPI::Texture*> hdriMapMipViews{};
		Array<RHI::RenderPassFrameBuffer*> hdriMapRTBs{};

		// Specular Convolution
		Array<RHI::TextureView*> specularConvolutionRTVs{};
		Array<RHI::RenderPassFrameBuffer*> specularConvolutionRTBs{};
		
		StaticArray<Array<RHI::TextureView*>, 6> cubeMapRTVMips{};
		StaticArray<Array<RHI::RenderPassFrameBuffer*>, 6> cubeMapRTBMips{};

		{
			RHI::RenderPassLayout rpLayout{};
			RHI::RenderPassAttachmentLayout colorAttachment{};
			colorAttachment.attachmentId = "Color";
			colorAttachment.format = hdriFormat;
			colorAttachment.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
			colorAttachment.loadAction = RHI::AttachmentLoadAction::Clear;
			colorAttachment.storeAction = RHI::AttachmentStoreAction::Store;
			colorAttachment.loadActionStencil = RHI::AttachmentLoadAction::DontCare;
			colorAttachment.storeActionStencil = RHI::AttachmentStoreAction::DontCare;
			colorAttachment.multisampleState.sampleCount = 1;

			rpLayout.attachmentLayouts.Add(colorAttachment);

			hdrRenderPass = RHI::gDynamicRHI->CreateRenderPass(rpLayout);

			for (int i = 0; i < 6; i++)
			{
				RHI::TextureViewDescriptor viewDesc{};
				viewDesc.texture = cubeMap;
				viewDesc.baseMipLevel = 0;
				viewDesc.mipLevelCount = 1;
				viewDesc.baseArrayLayer = i;
				viewDesc.arrayLayerCount = 1;
				viewDesc.dimension = RHI::Dimension::Dim2D;
				viewDesc.format = cubeMap->GetFormat();

				cubeMapRTVs[i] = RHI::gDynamicRHI->CreateTextureView(viewDesc);
				cubeMapRTBs[i] = RHI::gDynamicRHI->CreateRenderPassFrameBuffer({ hdrRenderPass, { cubeMapRTVs[i] } });

				viewDesc.texture = diffuseIrradianceCubeMap;
				irradianceCubeMapRTVs[i] = RHI::gDynamicRHI->CreateTextureView(viewDesc);
				irradianceCubeMapRTBs[i] = RHI::gDynamicRHI->CreateRenderPassFrameBuffer({ hdrRenderPass, { irradianceCubeMapRTVs[i] } });
			}

			for (int mip = 0; mip < hdriMap->GetMipLevelCount(); mip++)
			{
				RHI::TextureViewDescriptor viewDesc{};
				viewDesc.texture = hdriMap;
				viewDesc.baseMipLevel = mip;
				viewDesc.mipLevelCount = 1;
				viewDesc.baseArrayLayer = 0;
				viewDesc.arrayLayerCount = 1;
				viewDesc.dimension = RHI::Dimension::Dim2D;
				viewDesc.format = hdriMap->GetFormat();

				RHI::TextureView* textureView = RHI::gDynamicRHI->CreateTextureView(viewDesc);
				hdriMapMipViews.Add(new RPI::Texture(textureView));
				hdriMapRTBs.Add(RHI::gDynamicRHI->CreateRenderPassFrameBuffer({ hdrRenderPass, { textureView } }));
			}

			if (computeSpecularConvolution)
			{
				for (int mip = 0; mip < specularConvolution->GetMipLevelCount(); mip++)
				{
					RHI::TextureViewDescriptor viewDesc{};
					viewDesc.texture = specularConvolution;
					viewDesc.baseMipLevel = mip;
					viewDesc.mipLevelCount = 1;
					viewDesc.baseArrayLayer = 0;
					viewDesc.arrayLayerCount = 1;
					viewDesc.dimension = RHI::Dimension::Dim2D;
					viewDesc.format = specularConvolution->GetFormat();

					specularConvolutionRTVs.Add(RHI::gDynamicRHI->CreateTextureView(viewDesc));
					specularConvolutionRTBs.Add(RHI::gDynamicRHI->CreateRenderPassFrameBuffer({ hdrRenderPass, { specularConvolutionRTVs.Top() } }));
				}

				for (int face = 0; face < 6; face++)
				{
					for (int mip = 0; mip < cubeMap->GetMipLevelCount(); mip++)
					{
						RHI::TextureViewDescriptor viewDesc{};
						viewDesc.texture = cubeMap;
						viewDesc.baseMipLevel = mip;
						viewDesc.mipLevelCount = 1;
						viewDesc.baseArrayLayer = face;
						viewDesc.arrayLayerCount = 1;
						viewDesc.dimension = RHI::Dimension::Dim2D;
						viewDesc.format = cubeMap->GetFormat();

						cubeMapRTVMips[face].Add(RHI::gDynamicRHI->CreateTextureView(viewDesc));
						cubeMapRTBMips[face].Add(RHI::gDynamicRHI->CreateRenderPassFrameBuffer({ hdrRenderPass, { cubeMapRTVMips[face].Top() } }));
					}
				}
			}
		}

		RHI::RenderPass* grayscaleRenderTarget = nullptr;
		RHI::RenderPassFrameBuffer* grayscaleRTB = nullptr;
		RHI::RenderPassFrameBuffer* rowAverageRTB = nullptr;
		RHI::RenderPassFrameBuffer* columnAverageRTB = nullptr;
		RHI::RenderPassFrameBuffer* pdfJointRTB = nullptr;
		RHI::RenderPassFrameBuffer* pdfMarginalRTB = nullptr;
		RHI::RenderPassFrameBuffer* pdfConditionalRTB = nullptr;
		RHI::RenderPassFrameBuffer* cdfMarginalInverseRTB = nullptr;
		RHI::RenderPassFrameBuffer* cdfConditionalInverseRTB = nullptr;
		RHI::RenderPassFrameBuffer* diffuseConvolutionRTB = nullptr;

		if (diffuseIrradiance != nullptr)
		{
			RHI::RenderPassLayout rtLayout{};
			RHI::RenderPassAttachmentLayout colorAttachment{};
			colorAttachment.attachmentId = "Color";
			colorAttachment.format = RHI::Format::R16_SFLOAT;
			colorAttachment.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
			colorAttachment.loadAction = RHI::AttachmentLoadAction::Clear;
			colorAttachment.storeAction = RHI::AttachmentStoreAction::Store;
			colorAttachment.loadActionStencil = RHI::AttachmentLoadAction::DontCare;
			colorAttachment.storeActionStencil = RHI::AttachmentStoreAction::DontCare;
			colorAttachment.multisampleState.sampleCount = 1;

			rtLayout.attachmentLayouts.Add(colorAttachment);

			grayscaleRenderTarget = RHI::gDynamicRHI->CreateRenderPass(rtLayout);

			grayscaleRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { grayscale } });
			rowAverageRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { rowAverage } });
			columnAverageRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { columnAverage } });
			pdfJointRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { pdfJoint } });
			pdfMarginalRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { pdfMarginal } });
			pdfConditionalRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { pdfConditional } });
			cdfMarginalInverseRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { cdfMarginalInverse } });
			cdfConditionalInverseRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ grayscaleRenderTarget, { cdfConditionalInverse } });
			diffuseConvolutionRTB = gDynamicRHI->CreateRenderPassFrameBuffer({ hdrRenderPass, { diffuseIrradiance } });
		}

		defer(&)
		{
			delete hdrRenderPass;
			for (int i = 0; i < 6; i++)
			{
				for (int mip = 0; mip < cubeMapRTBMips[i].GetSize(); mip++)
				{
					delete cubeMapRTBMips[i][mip];
					delete cubeMapRTVMips[i][mip];
				}

				delete irradianceCubeMapRTVs[i];
				delete irradianceCubeMapRTBs[i];
				delete cubeMapRTVs[i];
				delete cubeMapRTBs[i];
			}

			for (auto mipMapRpi : specularConvolutionMipMaps)
			{
				delete mipMapRpi;
			}

			for (int i = 0; i < hdriMapMipViews.GetSize(); i++)
			{
				delete hdriMapMipViews[i];
				delete hdriMapRTBs[i];
			}

			for (int i = 0; i < specularConvolutionRTVs.GetSize(); i++)
			{
				delete specularConvolutionRTVs[i];
				delete specularConvolutionRTBs[i];
			}

			delete grayscaleRenderTarget;
			delete grayscaleRTB;
			delete rowAverageRTB;
			delete columnAverageRTB;
			delete pdfJointRTB;
			delete pdfMarginalRTB;
			delete pdfConditionalRTB;
			delete cdfMarginalInverseRTB;
			delete cdfConditionalInverseRTB;

			delete diffuseConvolutionRTB;
		};

		/////////////////////////////////////////////
		// - Materials & Data -

		static Matrix4x4 captureProjection = Matrix4x4::PerspectiveProjection(1.0f, 90.0f, 0.1f, 100.0f);

		// Order: right, left, top, bottom, front, back
		// TODO: Modify LookRotation entries after modifying the original function
		static Matrix4x4 captureViewMatrices[] = {
			Quat::LookRotation(Vec3(-1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)).ToMatrix(),
			Quat::LookRotation(Vec3(1.0f,  0.0f,  0.0f),  Vec3(0.0f, -1.0f,  0.0f)).ToMatrix(),
			Quat::LookRotation(Vec3(0.0f,  1.0f,  0.0f),  Vec3(0.0f,  0.0f, -1.0f)).ToMatrix(),
			Quat::LookRotation(Vec3(0.0f, -1.0f,  0.0f),  Vec3(0.0f,  0.0f,  1.0f)).ToMatrix(),
			Quat::LookRotation(Vec3(0.0f,  0.0f,  1.0f),  Vec3(0.0f, -1.0f,  0.0f)).ToMatrix(),
			Quat::LookRotation(Vec3(0.0f,  0.0f, -1.0f),  Vec3(0.0f, -1.0f,  0.0f)).ToMatrix()
		};

		RPI::Shader* equirectShader = desc.equirectangularShader;
		// 1 material for each face
		RPI::Material* equirectMaterials[6] = {};
		RPI::Material* equirectDiffuseIrradianceMaterials[6] = {};

		for (int i = 0; i < 6; i++)
		{
			equirectMaterials[i] = new RPI::Material(equirectShader);

			equirectMaterials[i]->SetPropertyValue("viewMatrix", captureViewMatrices[i]);
			equirectMaterials[i]->SetPropertyValue("projectionMatrix", captureProjection);
			equirectMaterials[i]->SetPropertyValue("viewProjectionMatrix", captureProjection * captureViewMatrices[i]);
			equirectMaterials[i]->SetPropertyValue("viewPosition", Vec4(0, 0, 0, 0));
			equirectMaterials[i]->SetPropertyValue("_InputSampler", linearSampler);
			equirectMaterials[i]->SetPropertyValue("_InputTexture", hdriMapRpi);
			equirectMaterials[i]->FlushProperties();

			if (diffuseIrradianceRpi != nullptr)
			{
				equirectDiffuseIrradianceMaterials[i] = new RPI::Material(equirectShader);

				equirectDiffuseIrradianceMaterials[i]->SetPropertyValue("viewMatrix", captureViewMatrices[i]);
				equirectDiffuseIrradianceMaterials[i]->SetPropertyValue("projectionMatrix", captureProjection);
				equirectDiffuseIrradianceMaterials[i]->SetPropertyValue("viewProjectionMatrix", captureProjection * captureViewMatrices[i]);
				equirectDiffuseIrradianceMaterials[i]->SetPropertyValue("viewPosition", Vec4(0, 0, 0, 0));
				equirectDiffuseIrradianceMaterials[i]->SetPropertyValue("_InputSampler", linearSampler);
				equirectDiffuseIrradianceMaterials[i]->SetPropertyValue("_InputTexture", diffuseIrradianceRpi);
				equirectDiffuseIrradianceMaterials[i]->FlushProperties();
			}
		}

		RPI::Material* grayscaleMaterial = new RPI::Material(desc.grayscaleShader);
		grayscaleMaterial->SetPropertyValue("_InputTexture", hdriMapRpi);
		grayscaleMaterial->SetPropertyValue("_InputSampler", linearSampler);
		grayscaleMaterial->FlushProperties();

		RPI::Material* rowAverageMaterial = new RPI::Material(desc.rowAverageShader);
		rowAverageMaterial->SetPropertyValue("_InputTexture", grayscaleRpi);
		rowAverageMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		rowAverageMaterial->FlushProperties();

		RPI::Material* columnAverageMaterial = new RPI::Material(desc.columnAverageShader);
		columnAverageMaterial->SetPropertyValue("_InputTexture", rowAverageRpi);
		columnAverageMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		columnAverageMaterial->FlushProperties();

		RPI::Material* pdfConditionalMaterial = new RPI::Material(desc.divisionShader);
		pdfConditionalMaterial->SetPropertyValue("_InputA", grayscaleRpi);
		pdfConditionalMaterial->SetPropertyValue("_InputB", rowAverageRpi);
		pdfConditionalMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		pdfConditionalMaterial->FlushProperties();

		RPI::Material* pdfMarginalMaterial = new RPI::Material(desc.divisionShader);
		pdfMarginalMaterial->SetPropertyValue("_InputA", rowAverageRpi);
		pdfMarginalMaterial->SetPropertyValue("_InputB", columnAverageRpi);
		pdfMarginalMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		pdfMarginalMaterial->FlushProperties();

		RPI::Material* cdfConditionalInverseMaterial = new RPI::Material(desc.cdfConditionalInverseShader);
		cdfConditionalInverseMaterial->SetPropertyValue("_InputTexture", pdfConditionalRpi);
		cdfConditionalInverseMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		cdfConditionalInverseMaterial->FlushProperties();

		RPI::Material* pdfJointMaterial = new RPI::Material(desc.divisionShader);
		pdfJointMaterial->SetPropertyValue("_InputA", grayscaleRpi);
		pdfJointMaterial->SetPropertyValue("_InputB", columnAverageRpi);
		pdfJointMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		pdfJointMaterial->FlushProperties();

		RPI::Material* cdfMarginalInverseMaterial = new RPI::Material(desc.cdfMarginalInverseShader);
		cdfMarginalInverseMaterial->SetPropertyValue("_InputTexture", pdfMarginalRpi);
		cdfMarginalInverseMaterial->SetPropertyValue("_InputSampler", nearestClamped);
		cdfMarginalInverseMaterial->FlushProperties();

		RPI::Material* diffuseConvolutionMaterial = new RPI::Material(desc.diffuseConvolutionShader);
		diffuseConvolutionMaterial->SetPropertyValue("_InputTexture", hdriMapRpi);
		diffuseConvolutionMaterial->SetPropertyValue("pdfJoint", pdfJointRpi);
		diffuseConvolutionMaterial->SetPropertyValue("cdfYInv", cdfMarginalInverseRpi);
		diffuseConvolutionMaterial->SetPropertyValue("cdfXInv", cdfConditionalInverseRpi);
		diffuseConvolutionMaterial->SetPropertyValue("_InputSampler", nearest);
		diffuseConvolutionMaterial->FlushProperties();

		Array<RPI::Material*> mipMapMaterials{};
		for (int i = 0; i < hdriMap->GetMipLevelCount(); i++)
		{
			if (i == 0)
			{
				mipMapMaterials.Add(nullptr);
				continue;
			}

			RPI::Material* mipMapMaterial = new RPI::Material(desc.mipMapShader);
			mipMapMaterial->SetPropertyValue("_InputTexture", hdriMapMipViews[i - 1]);
			mipMapMaterial->SetPropertyValue("_InputSampler", linearClampedSampler);
			mipMapMaterial->FlushProperties();

			mipMapMaterials.Add(mipMapMaterial);
		}

		Array<RPI::Material*> specularConvolutionMaterials{};
		for (int mip = 0; computeSpecularConvolution && mip < specularConvolution->GetMipLevelCount(); mip++)
		{
			int maxMipLevel = specularConvolution->GetMipLevelCount() - 1;
			if (maxMipLevel == 0)
				break; // Should never happen!

			RPI::Material* specularConvMaterial = new RPI::Material(desc.specularConvolutionShader);
			specularConvMaterial->SetPropertyValue("_InputTexture", hdriMapRpi);
			specularConvMaterial->SetPropertyValue("_InputSampler", linearSampler);
			specularConvMaterial->SetPropertyValue("_Roughness", (f32)mip / maxMipLevel);
			specularConvMaterial->FlushProperties();

			specularConvolutionMaterials.Add(specularConvMaterial);
		}

		StaticArray<Array<RPI::Material*>, 6> equirectToSpecularConvolutionMaterials{};
		if (specularConvolution != nullptr)
		{
			for (int face = 0; face < 6; face++)
			{
				for (int mip = 0; mip < specularConvolution->GetMipLevelCount(); mip++)
				{
					if (mip == 0)
					{
						equirectToSpecularConvolutionMaterials[face].Add(nullptr);
						continue;
					}

					RPI::Material* equirectSpecularMaterial = new RPI::Material(equirectShader);
					equirectSpecularMaterial->SetPropertyValue("viewMatrix", captureViewMatrices[face]);
					equirectSpecularMaterial->SetPropertyValue("projectionMatrix", captureProjection);
					equirectSpecularMaterial->SetPropertyValue("viewProjectionMatrix", captureProjection * captureViewMatrices[face]);
					equirectSpecularMaterial->SetPropertyValue("viewPosition", Vec4(0, 0, 0, 0));
					equirectSpecularMaterial->SetPropertyValue("_InputSampler", linearSampler);
					equirectSpecularMaterial->SetPropertyValue("_InputTexture", specularConvolutionMipMaps[mip]);
					equirectSpecularMaterial->FlushProperties();

					equirectToSpecularConvolutionMaterials[face].Add(equirectSpecularMaterial);
				}
			}
		}

		/////////////////////////////////////////////
		// - Command List Submission -

		RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
		RHI::CommandList* cmdList = RHI::gDynamicRHI->AllocateCommandList(queue);
		RHI::Fence* fence = RHI::gDynamicRHI->CreateFence(false);

		cmdList->Begin();
		{
			// - Load Input HDRI Image as a Flat 2D HDR texture -

			RHI::ResourceBarrierDescriptor barrier{};
			barrier.resource = hdriMap;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::CopyDestination;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			barrier.resource = outputBuffer;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::CopyDestination;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			{
				RHI::BufferToTextureCopy copyRegion{};
				copyRegion.srcBuffer = stagingBuffer;
				copyRegion.bufferOffset = 0;
				copyRegion.dstTexture = hdriMap;
				copyRegion.baseArrayLayer = 0;
				copyRegion.layerCount = 1;
				copyRegion.mipSlice = 0;

				cmdList->CopyTextureRegion(copyRegion);
			}

			barrier.resource = hdriMap;
			barrier.fromState = ResourceState::CopyDestination;
			barrier.toState = ResourceState::FragmentShaderResource;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			RHI::AttachmentClearValue clearValue{};
			clearValue.clearValue = Color(0, 0, 0, 1).ToVec4();

			barrier.resource = cubeMap;
			barrier.fromState = ResourceState::Undefined;
			barrier.toState = ResourceState::ColorOutput;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			// Convert equirectangular HDR flat image to HDR CubeMap
			for (int i = 0; i < 6; i++)
			{
				cmdList->BeginRenderPass(hdrRenderPass, cubeMapRTBs[i], &clearValue);

				RHI::ViewportState viewportState{};
				viewportState.x = viewportState.y = 0;
				viewportState.width = cubeMapRTVs[i]->GetTexture()->GetWidth();
				viewportState.height = cubeMapRTVs[i]->GetTexture()->GetHeight();
				viewportState.minDepth = 0;
				viewportState.maxDepth = 1;
				cmdList->SetViewports(1, &viewportState);

				RHI::ScissorState scissorState{};
				scissorState.x = scissorState.y = 0;
				scissorState.width = viewportState.width;
				scissorState.height = viewportState.height;
				cmdList->SetScissors(1, &scissorState);

				RHI::PipelineState* pipeline = equirectMaterials[i]->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
				cmdList->BindPipelineState(pipeline);

				cmdList->BindVertexBuffers(0, 1, &cubeVertexBufferView);
				cmdList->BindIndexBuffer(cubeMesh->indexBufferView);

				cmdList->SetShaderResourceGroup(equirectMaterials[i]->GetShaderResourceGroup());

				cmdList->CommitShaderResources();

				cmdList->DrawIndexed(cubeMesh->drawArguments.indexedArgs);

				cmdList->EndRenderPass();
			}

			barrier.resource = cubeMap;
			barrier.fromState = ResourceState::ColorOutput;
			barrier.toState = ResourceState::FragmentShaderResource;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			cmdList->ClearShaderResourceGroups();

			if (diffuseIrradiance != nullptr)
			{
				barrier.resource = grayscale;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				// Grayscale image
				cmdList->BeginRenderPass(grayscaleRenderTarget, grayscaleRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = grayscale->GetWidth();
					viewportState.height = grayscale->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = grayscaleMaterial->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(grayscaleMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				barrier.resource = grayscale;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = rowAverage;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = columnAverage;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = pdfConditional;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = pdfMarginal;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = cdfConditionalInverse;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = pdfJoint;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = cdfMarginalInverse;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				cmdList->ClearShaderResourceGroups();

				// Row Average
				cmdList->BeginRenderPass(grayscaleRenderTarget, rowAverageRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = rowAverage->GetWidth();
					viewportState.height = rowAverage->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = rowAverageMaterial->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(rowAverageMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				barrier.resource = rowAverage;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				///////////////////////////////////////////////////////////////////////

				// Column Average
				cmdList->BeginRenderPass(grayscaleRenderTarget, columnAverageRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = columnAverage->GetWidth();
					viewportState.height = columnAverage->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = columnAverageMaterial->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(columnAverageMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				// PDF Conditional
				cmdList->BeginRenderPass(grayscaleRenderTarget, pdfConditionalRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = pdfConditional->GetWidth();
					viewportState.height = pdfConditional->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = pdfConditionalMaterial->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(pdfConditionalMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				barrier.resource = columnAverage;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = pdfConditional;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				///////////////////////////////////////////////////////////////////////

				// PDF Marginal
				cmdList->BeginRenderPass(grayscaleRenderTarget, pdfMarginalRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = pdfMarginal->GetWidth();
					viewportState.height = pdfMarginal->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = pdfMarginalMaterial->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(pdfMarginalMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				// CDF Conditional Inverse
				cmdList->BeginRenderPass(grayscaleRenderTarget, cdfConditionalInverseRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = cdfConditionalInverse->GetWidth();
					viewportState.height = cdfConditionalInverse->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = cdfConditionalInverseMaterial->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(cdfConditionalInverseMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				// PDF Joint
				cmdList->BeginRenderPass(grayscaleRenderTarget, pdfJointRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = pdfJoint->GetWidth();
					viewportState.height = pdfJoint->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = pdfJointMaterial->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(pdfJointMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				barrier.resource = pdfMarginal;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = cdfConditionalInverse;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = pdfJoint;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				///////////////////////////////////////////////////////////////////////

				// CDF Marginal Inverse

				cmdList->BeginRenderPass(grayscaleRenderTarget, cdfMarginalInverseRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = cdfMarginalInverse->GetWidth();
					viewportState.height = cdfMarginalInverse->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = cdfMarginalInverseMaterial->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(cdfMarginalInverseMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				barrier.resource = cdfMarginalInverse;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				///////////////////////////////////////////////////////////////////////

				// Diffuse Convolution

				barrier.resource = diffuseIrradiance;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				cmdList->BeginRenderPass(hdrRenderPass, diffuseConvolutionRTB, &clearValue);
				{
					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = diffuseIrradiance->GetWidth();
					viewportState.height = diffuseIrradiance->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = diffuseConvolutionMaterial->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(diffuseConvolutionMaterial->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);
				}
				cmdList->EndRenderPass();

				cmdList->ClearShaderResourceGroups();

				barrier.resource = diffuseIrradiance;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				/////////////////////////////////////////////////////////////////////////////

				// Diffuse Irradiance CubeMap

				barrier.resource = diffuseIrradianceCubeMap;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				for (int i = 0; i < 6; i++)
				{
					cmdList->BeginRenderPass(hdrRenderPass, irradianceCubeMapRTBs[i], &clearValue);

					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = diffuseIrradianceCubeMap->GetWidth();
					viewportState.height = diffuseIrradianceCubeMap->GetHeight();
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = equirectDiffuseIrradianceMaterials[i]->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, 1, &cubeVertexBufferView);
					cmdList->BindIndexBuffer(cubeMesh->indexBufferView);

					cmdList->SetShaderResourceGroup(equirectDiffuseIrradianceMaterials[i]->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawIndexed(cubeMesh->drawArguments.indexedArgs);

					cmdList->EndRenderPass();
				}

				barrier.resource = diffuseIrradianceCubeMap;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::CopySource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				for (int i = 0; i < 6; i++)
				{
					RHI::TextureToBufferCopy copyRegion{};
					copyRegion.srcTexture = diffuseIrradianceCubeMap;
					copyRegion.mipSlice = 0;
					copyRegion.baseArrayLayer = i;
					copyRegion.layerCount = 1;
					copyRegion.dstBuffer = diffuseIrradianceOutputBuffer;
					copyRegion.bufferOffset = i * diffuseIrradianceCubeMap->GetWidth() * diffuseIrradianceCubeMap->GetHeight() *
						GetBitsPerPixelForFormat(cubeMapDesc.format) / 8;

					cmdList->CopyTextureRegion(copyRegion);
				}
			}

			/////////////////////////////////////////////////////////////////////////////
			// - Generate MipMaps -

			barrier.resource = hdriMap;
			barrier.fromState = ResourceState::FragmentShaderResource;
			barrier.toState = ResourceState::ColorOutput;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.baseMipLevel = 1;
			barrier.subresourceRange.levelCount = hdriMap->GetMipLevelCount() - 1;
			cmdList->ResourceBarrier(1, &barrier);

			for (int i = 1; i < hdriMap->GetMipLevelCount(); i++)
			{
				// Generate mip level 'i'
				cmdList->BeginRenderPass(hdrRenderPass, hdriMapRTBs[i], &clearValue);
				RHI::ViewportState viewportState{};
				viewportState.x = viewportState.y = 0;
				viewportState.width = hdriMap->GetWidth(i);
				viewportState.height = hdriMap->GetHeight(i);
				viewportState.minDepth = 0;
				viewportState.maxDepth = 1;
				cmdList->SetViewports(1, &viewportState);

				RHI::ScissorState scissorState{};
				scissorState.x = scissorState.y = 0;
				scissorState.width = viewportState.width;
				scissorState.height = viewportState.height;
				cmdList->SetScissors(1, &scissorState);

				RHI::PipelineState* pipeline = mipMapMaterials[i]->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
				cmdList->BindPipelineState(pipeline);

				cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

				cmdList->SetShaderResourceGroup(mipMapMaterials[i]->GetShaderResourceGroup());

				cmdList->CommitShaderResources();

				cmdList->DrawLinear(fullscreenQuadArgs);

				cmdList->EndRenderPass();

				barrier.resource = hdriMap;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.subresourceRange.baseMipLevel = i;
				barrier.subresourceRange.levelCount = 1;
				cmdList->ResourceBarrier(1, &barrier);
			}

			/////////////////////////////////////////////////////////////////////////////
			// - Specular Convolution -

			if (computeSpecularConvolution)
			{
				barrier.resource = specularConvolution;
				barrier.fromState = ResourceState::Undefined;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				for (int mip = 1; mip < specularConvolution->GetMipLevelCount(); mip++)
				{
					cmdList->BeginRenderPass(hdrRenderPass, specularConvolutionRTBs[mip], &clearValue);

					RHI::ViewportState viewportState{};
					viewportState.x = viewportState.y = 0;
					viewportState.width = specularConvolution->GetWidth(mip);
					viewportState.height = specularConvolution->GetHeight(mip);
					viewportState.minDepth = 0;
					viewportState.maxDepth = 1;
					cmdList->SetViewports(1, &viewportState);

					RHI::ScissorState scissorState{};
					scissorState.x = scissorState.y = 0;
					scissorState.width = viewportState.width;
					scissorState.height = viewportState.height;
					cmdList->SetScissors(1, &scissorState);

					RHI::PipelineState* pipeline = specularConvolutionMaterials[mip]->GetCurrentShader()->GetDefaultPipeline();
					cmdList->BindPipelineState(pipeline);

					cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

					cmdList->SetShaderResourceGroup(specularConvolutionMaterials[mip]->GetShaderResourceGroup());

					cmdList->CommitShaderResources();

					cmdList->DrawLinear(fullscreenQuadArgs);

					cmdList->EndRenderPass();
				}

				barrier.resource = specularConvolution;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				barrier.resource = cubeMap;
				barrier.fromState = ResourceState::FragmentShaderResource;
				barrier.toState = ResourceState::ColorOutput;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);

				for (int mip = 1; mip < cubeMap->GetMipLevelCount(); mip++)
				{
					for (int face = 0; face < 6; face++)
					{
						cmdList->BeginRenderPass(hdrRenderPass, cubeMapRTBMips[face][mip], &clearValue);
						
						RHI::ViewportState viewportState{};
						viewportState.x = viewportState.y = 0;
						viewportState.width = cubeMap->GetWidth(mip);
						viewportState.height = cubeMap->GetHeight(mip);
						viewportState.minDepth = 0;
						viewportState.maxDepth = 1;
						cmdList->SetViewports(1, &viewportState);

						RHI::ScissorState scissorState{};
						scissorState.x = scissorState.y = 0;
						scissorState.width = viewportState.width;
						scissorState.height = viewportState.height;
						cmdList->SetScissors(1, &scissorState);

						RHI::PipelineState* pipeline = equirectToSpecularConvolutionMaterials[face][mip]->GetCurrentShader()->GetDefaultPipeline();
						cmdList->BindPipelineState(pipeline);

						cmdList->BindVertexBuffers(0, 1, &cubeVertexBufferView);
						cmdList->BindIndexBuffer(cubeMesh->indexBufferView);

						cmdList->SetShaderResourceGroup(equirectToSpecularConvolutionMaterials[face][mip]->GetShaderResourceGroup());

						cmdList->CommitShaderResources();

						cmdList->DrawIndexed(cubeMesh->drawArguments.indexedArgs);

						cmdList->EndRenderPass();
					}
				}

				barrier.resource = cubeMap;
				barrier.fromState = ResourceState::ColorOutput;
				barrier.toState = ResourceState::FragmentShaderResource;
				barrier.subresourceRange = RHI::SubresourceRange::All();
				cmdList->ResourceBarrier(1, &barrier);
			}

			/////////////////////////////////////////////////////////////////////////////
			// - Copy CubeMap to buffer -

			barrier.resource = cubeMap;
			barrier.fromState = ResourceState::FragmentShaderResource;
			barrier.toState = ResourceState::CopySource;
			barrier.subresourceRange = RHI::SubresourceRange::All();
			cmdList->ResourceBarrier(1, &barrier);

			u64 copyBufferOffset = 0;

			for (int mip = 0; mip < cubeMap->GetMipLevelCount(); mip++)
			{
				RHI::TextureToBufferCopy copyRegion{};
				copyRegion.srcTexture = cubeMap;
				copyRegion.mipSlice = mip;
				copyRegion.baseArrayLayer = 0;
				copyRegion.layerCount = 6;
				copyRegion.dstBuffer = outputBuffer;
				copyRegion.bufferOffset = copyBufferOffset;

				copyBufferOffset += cubeMap->GetWidth() / (u32)pow(2, mip) * cubeMap->GetHeight() / (u32)pow(2, mip) * 
					GetBitsPerPixelForFormat(cubeMapDesc.format) / 8 * 6;

				cmdList->CopyTextureRegion(copyRegion);
			}

			barrier.resource = cubeMap;
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

		/////////////////////////////////////////////
		// - Output Data -

		outputBuffer->Map(0, outputBuffer->GetBufferSize(), &dataPtr);
		{
			if (!desc.useCompression)
			{
				output.Reserve(outputBuffer->GetBufferSize());
				memcpy(output.GetDataPtr(), dataPtr, outputBuffer->GetBufferSize());
			}
			else // BC6H compression
			{
				int numPixelsPerFace = cubeMapRes * cubeMapRes;

				// BC6H uses 1 byte (8 bits) per pixel
				u64 totalCompressedSize = CalculateTotalTextureSize(cubeMapRes, cubeMapRes, 8, 6, cubeMap->GetMipLevelCount());
				output.Reserve(totalCompressedSize);
				void* outputDataPtr = output.GetDataPtr();

				Array<Thread> threads{};

				for (int mip = 0; mip < cubeMap->GetMipLevelCount(); mip++)
				{
					for (int face = 0; face < 6; face++)
					{
						threads.EmplaceBack([dataPtr, mip, face, outputDataPtr, cubeMapRes, cubeMapDesc]
							{
								u64 totalSizeTillCurrentMipLevel = 
									CalculateTotalTextureSize(cubeMapRes, cubeMapRes, GetBitsPerPixelForFormat(cubeMapDesc.format), 6, mip);
								u32 currentResolution = cubeMapRes / (u32)pow(2, mip);

								u32 compressedByteSizePerFace = currentResolution * currentResolution * 8 / 8; // BC6H uses 8 bits per pixel
								u64 totalCompressedSizeTillCurrentMipLevel =
									CalculateTotalTextureSize(cubeMapRes, cubeMapRes, 8, 6, mip); // BC6H uses 8 bits per pixel

								CMImage image =
									CMImage::LoadRawImageFromMemory((unsigned char*)dataPtr + totalSizeTillCurrentMipLevel +
											face * currentResolution * currentResolution * GetBitsPerPixelForFormat(cubeMapDesc.format) / 8,
										currentResolution, currentResolution,
										CMImageFormat::RGBA16, CMImageSourceFormat::None, 
										GetBitsPerPixelForFormat(cubeMapDesc.format) / 4, 
										GetBitsPerPixelForFormat(cubeMapDesc.format));

								CMImageEncoder encoder{};

								bool result = encoder.EncodeToBCn(image, (u8*)outputDataPtr + totalCompressedSizeTillCurrentMipLevel +
									compressedByteSizePerFace * face,
									CMImageSourceFormat::BC6H);
								if (!result)
								{
									CE_LOG(Error, All, "BC6H encoding failed: {}", encoder.GetErrorMessage());
								}
							});
					}
				}

				for (int i = 0; i < threads.GetSize(); i++)
				{
					if (threads[i].IsJoinable())
						threads[i].Join();
				}
			}
		}
		outputBuffer->Unmap();

		if (diffuseIrradiance != nullptr && desc.diffuseIrradianceOutput != nullptr)
		{
			diffuseIrradianceOutputBuffer->Map(0, diffuseIrradianceOutputBuffer->GetBufferSize(), &dataPtr);
			{
				if (!desc.compressDiffuseIrradiance)
				{
					desc.diffuseIrradianceOutput->Reserve(diffuseIrradianceOutputBuffer->GetBufferSize());
					memcpy(desc.diffuseIrradianceOutput->GetDataPtr(), dataPtr, diffuseIrradianceOutputBuffer->GetBufferSize());
				}
				else // BC6H compression
				{
					int numPixelsPerFace = diffuseIrradianceCubeMap->GetWidth() * diffuseIrradianceCubeMap->GetHeight();
					u64 compressedByteSizePerFace = numPixelsPerFace; // BC6H uses 1 byte per pixel
					desc.diffuseIrradianceOutput->Reserve(compressedByteSizePerFace * 6);
					void* outputDataPtr = desc.diffuseIrradianceOutput->GetDataPtr();

					Array<Thread> threads{};

					for (int face = 0; face < 6; face++)
					{
						threads.EmplaceBack([outputDataPtr, dataPtr, compressedByteSizePerFace, face, numPixelsPerFace, diffuseIrradianceCubeMap, cubeMapDesc]
							{
								CMImage image =
									CMImage::LoadRawImageFromMemory((unsigned char*)dataPtr + face * numPixelsPerFace * GetBitsPerPixelForFormat(cubeMapDesc.format) / 8,
										diffuseIrradianceCubeMap->GetWidth(), diffuseIrradianceCubeMap->GetHeight(),
										CMImageFormat::RGBA16, CMImageSourceFormat::None, 16, 64);

								CMImageEncoder encoder{};

								bool result = encoder.EncodeToBCn(image, (u8*)outputDataPtr + compressedByteSizePerFace * face, CMImageSourceFormat::BC6H);
								if (!result)
								{
									CE_LOG(Error, All, "BC6H encoding failed: {}", encoder.GetErrorMessage());
								}
							});
					}

					for (int face = 0; face < 6; face++)
					{
						if (threads[face].IsJoinable())
							threads[face].Join();
					}
				}
			}
			diffuseIrradianceOutputBuffer->Unmap();
		}

		/////////////////////////////////////////////
		// - Free up memory -

		RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);
		RHI::gDynamicRHI->DestroyFence(fence);

		for (int i = 0; i < 6; i++)
		{
			delete equirectMaterials[i];
			equirectMaterials[i] = nullptr;
			delete equirectDiffuseIrradianceMaterials[i];
			equirectDiffuseIrradianceMaterials[i] = nullptr;
		}

		for (int i = 0; i < mipMapMaterials.GetSize(); i++)
		{
			delete mipMapMaterials[i];
		}
		for (int i = 0; i < specularConvolutionMaterials.GetSize(); i++)
		{
			delete specularConvolutionMaterials[i];
		}
		for (int face = 0; face < 6; face++)
		{
			for (int mip = 0; mip < equirectToSpecularConvolutionMaterials[face].GetSize(); mip++)
			{
				delete equirectToSpecularConvolutionMaterials[face][mip];
			}
			equirectToSpecularConvolutionMaterials[face].Clear();
		}

		delete grayscaleMaterial;
		delete rowAverageMaterial;
		delete columnAverageMaterial;
		delete pdfConditionalMaterial;
		delete pdfMarginalMaterial;
		delete cdfConditionalInverseMaterial;
		delete pdfJointMaterial;
		delete cdfMarginalInverseMaterial;
		delete diffuseConvolutionMaterial;

		delete cubeMapRpi;
		delete hdriMapRpi;
		delete diffuseIrradianceRpi;
		delete diffuseIrradianceCubeMap;
		delete specularConvolutionRpi;
		delete outputBuffer; delete diffuseIrradianceOutputBuffer;
		delete cubeModel;
		delete stagingBuffer;
		delete grayscaleRpi;
		delete rowAverageRpi;
		delete columnAverageRpi;
		delete pdfJointRpi;
		delete pdfMarginalRpi;
		delete pdfConditionalRpi;
		delete cdfMarginalInverseRpi;
		delete cdfConditionalInverseRpi;

		return true;
	}

    
} // namespace CE::RPI
