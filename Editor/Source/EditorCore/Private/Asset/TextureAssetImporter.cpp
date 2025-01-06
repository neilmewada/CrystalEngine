#include "EditorCore.h"

namespace CE::Editor
{

	Array<AssetImportJob*> TextureAssetImporter::CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets)
	{
		Array<AssetImportJob*> jobs{};

		for (int i = 0; i < sourceAssets.GetSize(); i++)
		{
			auto job = new TextureAssetImportJob(this, sourceAssets[i], productAssets[i]);
			job->compressionQuality = compressionQuality;
			job->ignoreCompressionFolders = ignoreCompressionFolders;
			job->anisotropy = anisotropy;
			job->importHdrAsCubemap = importHdrAsCubemap;
			job->convoluteCubemap = convoluteCubemap;
			job->diffuseConvolutionResolution = diffuseConvolutionResolution;
			job->compressConvolution = compressConvolution;
			job->specularConvolution = specularConvolution;
			
			jobs.Add(job);
		}

		return jobs;
	}

	Array<Name> TextureAssetImporter::GetProductAssetDependencies()
	{
		static const Array<Name> dependencies{
			"/Editor/Assets/Shaders/CubeMap/Equirectangular",
			"/Editor/Assets/Shaders/CubeMap/IBL",
			"/Editor/Assets/Shaders/CubeMap/IBLConvolution",
			"/Editor/Assets/Shaders/Utils/MipMapGen"
		};

		return dependencies;
	}

	bool TextureAssetImportJob::ProcessAsset(const Ref<Bundle>& bundle)
	{
		if (bundle == nullptr)
			return false;
		if (!sourcePath.Exists())
			return false;

		// 1. Retrieve original Uuids

		Uuid cubeMapUuid = Uuid::Random();
		Uuid diffuseUuid = Uuid::Random();
		Uuid textureUuid = Uuid::Random();

		if (!regenerateUuid)
		{
			for (int i = 0; i < bundle->GetSubObjectCount(); ++i)
			{
				Ref<Object> subObject = bundle->GetSubObject(i);

				if (subObject->IsOfType<TextureCube>())
				{
					cubeMapUuid = subObject->GetUuid();

					for (int j = 0; j < subObject->GetSubObjectCount(); ++j)
					{
						if (subObject->GetSubObject(j)->IsOfType<TextureCube>())
						{
							diffuseUuid = Uuid::Random();
							break;
						}
					}

					break;
				}
			}

			for (int i = 0; i < bundle->GetSubObjectCount(); ++i)
			{
				Ref<Object> subObject = bundle->GetSubObject(i);

				if (subObject->IsOfType<Texture2D>())
				{
					textureUuid = subObject->GetUuid();

					break;
				}
			}
		}

		// 2. Clear the bundle of any subobjects/assets, we will build the asset from scratch
		bundle->DestroyAllSubObjects();

		// 3. Check if compression is force disabled for this path

		bool forceUncompressed = false;
		String relativeProductPath = "";

		if (IO::Path::IsSubDirectory(productPath.GetParentPath(), EngineDirectories::GetEngineInstallDirectory() / "Engine"))
		{
			relativeProductPath = IO::Path::GetRelative(productPath.GetParentPath(), EngineDirectories::GetEngineInstallDirectory());
		}
		else if (IO::Path::IsSubDirectory(productPath.GetParentPath(), EngineDirectories::GetEngineInstallDirectory() / "Editor"))
		{
			relativeProductPath = IO::Path::GetRelative(productPath.GetParentPath(), EngineDirectories::GetEngineInstallDirectory());
		}
		else if (IO::Path::IsSubDirectory(productPath.GetParentPath(), EngineDirectories::GetGameDirectory()))
		{
			relativeProductPath = IO::Path::GetRelative(productPath.GetParentPath(), EngineDirectories::GetGameDirectory());
		}

		relativeProductPath = relativeProductPath.Replace({ '\\' }, '/');
		if (relativeProductPath.NotEmpty())
		{
			if (relativeProductPath[0] != '/')
			{
				relativeProductPath.InsertAt('/', 0);
			}

			for (const auto& ignoreCompressionFolder : ignoreCompressionFolders)
			{
				if (relativeProductPath.StartsWith(ignoreCompressionFolder))
				{
					forceUncompressed = true;
					break;
				}
			}
		}

		// 4. Find valid file name to use as object name

		String extension = sourcePath.GetFileName().GetExtension().GetString();
		String fileName = sourcePath.GetFileName().RemoveExtension().GetString();
		// Make sure we can use the fileName as name of an object
		fileName = FixObjectName(fileName);

		// 5. Load raw image

		CMImage image = CMImage::LoadFromFile(sourcePath);
		if (!image.IsValid())
			return false;

		defer(&)
		{
			image.Free();
		};

		// 6. Check if it's a CubeMap
		
		bool isCubeMap = false;
		if (extension == ".hdr" && image.GetWidth() == image.GetHeight() * 2)
		{
			isCubeMap = importHdrAsCubemap;
		}

		// 7. Find valid compression format for the pixel format

		CMImageFormat imageFormat = image.GetFormat();
		CMImageSourceFormat targetSourceFormat = CMImageSourceFormat::None;

		TextureFormat pixelFormat = TextureFormat::None;
		TextureSourceCompressionFormat compressionFormat = TextureSourceCompressionFormat::None;
		
		switch (imageFormat)
		{
		case CMImageFormat::R8:
			pixelFormat = TextureFormat::R8;
			break;
		case CMImageFormat::RG8:
			pixelFormat = TextureFormat::RG8;
			break;
		case CMImageFormat::RGB8:
			pixelFormat = TextureFormat::RGB8;
			break;
		case CMImageFormat::RGBA8:
			pixelFormat = TextureFormat::RGBA8;
			break;
		case CMImageFormat::RG32:
			pixelFormat = TextureFormat::RGFloat;
			break;
		case CMImageFormat::RG16:
			pixelFormat = TextureFormat::RGHalf;
			break;
		case CMImageFormat::RGB16:
		case CMImageFormat::RGBA16:
			pixelFormat = TextureFormat::RGBAHalf;
			break;
		case CMImageFormat::RGB32:
		case CMImageFormat::RGBA32:
			pixelFormat = TextureFormat::RGBAFloat;
			break;
		}
		
		if (!forceUncompressed && compressionQuality != TextureCompressionQuality::None && PlatformMisc::IsDesktopPlatform(targetPlatform))
		{
			switch (imageFormat)
			{
			case CMImageFormat::R8:
				pixelFormat = TextureFormat::BC4;
				targetSourceFormat = CMImageSourceFormat::BC4;
				compressionFormat = TextureSourceCompressionFormat::BC4;
				break;
			case CMImageFormat::RG8:
				pixelFormat = TextureFormat::BC5;
				targetSourceFormat = CMImageSourceFormat::BC5;
				compressionFormat = TextureSourceCompressionFormat::BC5;
				break;
			case CMImageFormat::RGB8:
				if (compressionQuality == TextureCompressionQuality::High)
				{
					pixelFormat = TextureFormat::BC7;
					targetSourceFormat = CMImageSourceFormat::BC7;
					compressionFormat = TextureSourceCompressionFormat::BC7;
				}
				else
				{
					pixelFormat = TextureFormat::BC1;
					targetSourceFormat = CMImageSourceFormat::BC1;
					compressionFormat = TextureSourceCompressionFormat::BC1;
				}
				break;
			case CMImageFormat::RGBA8:
				if (compressionQuality == TextureCompressionQuality::High)
				{
					pixelFormat = TextureFormat::BC7;
					targetSourceFormat = CMImageSourceFormat::BC7;
					compressionFormat = TextureSourceCompressionFormat::BC7;
				}
				else
				{
					pixelFormat = TextureFormat::BC3;
					targetSourceFormat = CMImageSourceFormat::BC3;
					compressionFormat = TextureSourceCompressionFormat::BC3;
				}
				break;
			case CMImageFormat::RGB32:
			case CMImageFormat::RGBA32:
				pixelFormat = TextureFormat::BC6H;
				targetSourceFormat = CMImageSourceFormat::BC6H;
				targetSourceFormat = CMImageSourceFormat::BC6H;
				compressionFormat = TextureSourceCompressionFormat::BC6H;
				break;
			}
		}

		// 8. Do the actual processing
		
		if (isCubeMap) // Process CubeMap
		{
			return ProcessCubeMap(fileName, bundle.Get(), image, 
				pixelFormat, compressionFormat, targetSourceFormat,
				cubeMapUuid, diffuseUuid);
		}
		else
		{
			return ProcessTex2D(fileName, bundle.Get(), image, 
				pixelFormat, compressionFormat, targetSourceFormat,
				textureUuid);
		}
	}

	bool TextureAssetImportJob::ProcessCubeMap(const String& name, Bundle* bundle, const CMImage& sourceImage, TextureFormat pixelFormat,
		TextureSourceCompressionFormat compressionFormat,
		CMImageSourceFormat targetSourceFormat,
		Uuid cubeMapUuid, Uuid diffuseUuid)
	{
		TextureCube* texture = CreateObject<TextureCube>(bundle, name, OF_NoFlags,
			TextureCube::StaticClass(), nullptr, cubeMapUuid);

		// Temporary code
		
		if (compressionFormat == TextureSourceCompressionFormat::None)
		{
			pixelFormat = TextureFormat::RGBAHalf;
		}

		texture->anisoLevel = 0;
		texture->width = texture->height = sourceImage.GetHeight();
		texture->mipLevels = 1;
		texture->addressModeU = texture->addressModeV = TextureAddressMode::Repeat;
		texture->filter = RHI::FilterMode::Linear;
		texture->pixelFormat = pixelFormat;
		texture->compressionQuality = compressionQuality;
		texture->sourceCompressionFormat = compressionFormat;

		Ref<CE::Shader> equirectShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/CubeMap/Equirectangular");
		Ref<CE::Shader> iblShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/CubeMap/IBL");
		Ref<CE::Shader> iblConvolutionShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/CubeMap/IBLConvolution");
		Ref<CE::Shader> mipmapShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/Utils/MipMapGen");

		ShaderCollection* equirectShaderCollection = equirectShader->GetShaderCollection();
		ShaderCollection* iblShaderCollection = iblShader->GetShaderCollection();
		ShaderCollection* iblConvolutionShaderCollection = iblConvolutionShader->GetShaderCollection();
		ShaderCollection* mipMapShaderCollection = mipmapShader->GetShaderCollection();

		RPI::CubeMapOfflineProcessInfo processInfo{};
		processInfo.name = name;
		processInfo.sourceImage = sourceImage;
		processInfo.equirectangularShader = equirectShaderCollection->At(0).shader;
		processInfo.grayscaleShader = iblShaderCollection->At(0).shader;
		processInfo.rowAverageShader = iblShaderCollection->At(1).shader;
		processInfo.columnAverageShader = iblShaderCollection->At(2).shader;
		processInfo.divisionShader = iblShaderCollection->At(3).shader;
		processInfo.cdfMarginalInverseShader = iblShaderCollection->At(4).shader;
		processInfo.cdfConditionalInverseShader = iblShaderCollection->At(5).shader;
		processInfo.diffuseConvolutionShader = iblConvolutionShaderCollection->At(0).shader;
		processInfo.useCompression = (compressionFormat == TextureSourceCompressionFormat::BC6H);
		processInfo.diffuseIrradianceResolution = convoluteCubemap ? diffuseConvolutionResolution : 0;
		processInfo.diffuseIrradianceOutput = nullptr;
		processInfo.compressDiffuseIrradiance = compressConvolution;
		processInfo.specularConvolution = specularConvolution;
		processInfo.specularConvolutionShader = iblConvolutionShaderCollection->At(1).shader;
		processInfo.mipMapShader = mipMapShaderCollection->At(0).shader;

		if (convoluteCubemap && diffuseConvolutionResolution > 0)
		{

			TextureCube* diffuseConvolution = CreateObject<TextureCube>(texture, name + "_Diffuse",
				OF_NoFlags, TextureCube::StaticClass(), nullptr, diffuseUuid);

			diffuseConvolution->anisoLevel = 0;
			diffuseConvolution->width = diffuseConvolution->height = diffuseConvolutionResolution;
			diffuseConvolution->mipLevels = 1;
			diffuseConvolution->addressModeU = diffuseConvolution->addressModeV = TextureAddressMode::Repeat;
			diffuseConvolution->filter = RHI::FilterMode::Linear;
			diffuseConvolution->pixelFormat = pixelFormat;//TextureFormat::RGBAHalf;
			diffuseConvolution->compressionQuality = compressionQuality;
			diffuseConvolution->sourceCompressionFormat = compressionFormat;//TextureSourceCompressionFormat::None;

			if (!compressConvolution)
			{
				diffuseConvolution->pixelFormat = TextureFormat::RGBAHalf;
				diffuseConvolution->sourceCompressionFormat = TextureSourceCompressionFormat::None;
			}
			else
			{
				diffuseConvolution->pixelFormat = TextureFormat::BC6H;
				diffuseConvolution->sourceCompressionFormat = TextureSourceCompressionFormat::BC6H;
			}

			processInfo.diffuseIrradianceOutput = &diffuseConvolution->source;
			texture->diffuseConvolution = diffuseConvolution;
		}

		RPI::CubeMapProcessor processor{};
		
		bool result = processor.ProcessCubeMapOffline(processInfo, texture->source, texture->mipLevels);
		return result;
	}

	bool TextureAssetImportJob::ProcessTex2D(const String& name, Bundle* bundle, const CMImage& image, TextureFormat pixelFormat,
		TextureSourceCompressionFormat compressionFormat,
		CMImageSourceFormat targetSourceFormat,
		Uuid textureUuid)
	{
		Ref<Texture2D> texture = CreateObject<Texture2D>(bundle, name,
			OF_NoFlags, Texture2D::StaticClass(), nullptr, textureUuid);

		texture->anisoLevel = anisotropy;
		texture->width = image.GetWidth();
		texture->height = image.GetHeight();
		texture->arrayCount = 1;
		texture->mipLevels = 1;
		texture->addressModeU = texture->addressModeV = TextureAddressMode::Repeat;
		texture->filter = RHI::FilterMode::Linear;
		texture->pixelFormat = pixelFormat;
		texture->compressionQuality = compressionQuality;
		texture->sourceCompressionFormat = compressionFormat;

		if (targetSourceFormat != CMImageSourceFormat::None) // Use BCn format
		{
			float quality = 0.05f;
			CMImageEncoder encoder{};
			u64 reserveSize = encoder.GetCompressedSizeRequirement(image, targetSourceFormat);
			if (reserveSize == 0)
			{
				errorMessage = "Invalid compression format!";
				return false;
			}

			texture->source.Reserve(reserveSize);

			bool success = encoder.EncodeToBCn(image, texture->source.GetDataPtr(), targetSourceFormat, CMImageEncoder::Quality_Normal);
			if (!success)
			{
				errorMessage = "Failed to encode to BCn format!";
				return false;
			}
		}
		else // Store raw pixels
		{
			MemoryStream stream = MemoryStream(image.GetDataSize(), true);
			stream.SetBinaryMode(true);
			stream.SetAutoResizeIncrement(4_MB);

			u32 width = image.GetWidth();
			u32 height = image.GetHeight();

			u32 numPixels = width * height;
			u32 numChannels = image.GetNumChannels();
			u32 channelStride = numChannels;
			if (numChannels == 3)
				channelStride = 4;

			bool isFloat = false;
			switch (image.GetFormat())
			{
			case CMImageFormat::R32:
			case CMImageFormat::RG32:
			case CMImageFormat::RGB32:
			case CMImageFormat::RGBA32:
				isFloat = true;
				break;
			}

			for (int i = 0; i < numPixels; i++)
			{
				if (isFloat)
				{
					f32 r = *((f32*)image.GetDataPtr() + channelStride * i + 0);
					f32 g = 0;
					if (channelStride >= 2)
						g = *((f32*)image.GetDataPtr() + channelStride * i + 1);
					f32 b = 0;
					if (channelStride >= 3)
						b = *((f32*)image.GetDataPtr() + channelStride * i + 2);
					f32 a = 0;
					if (channelStride >= 4)
						a = *((f32*)image.GetDataPtr() + channelStride * i + 3);

					if (compressionQuality == TextureCompressionQuality::None) // Store full float32
					{
						stream << r;
						if (numChannels >= 2)
							stream << g;
						if (numChannels >= 3)
							stream << b;
						if (numChannels >= 4)
							stream << a;
					}
					else // Store as float16 (half)
					{
						stream << Math::ToFloat16(r);
						if (numChannels >= 2)
							stream << Math::ToFloat16(g);
						if (numChannels >= 3)
							stream << Math::ToFloat16(b);
						if (numChannels >= 4)
							stream << Math::ToFloat16(a);
					}
				}
				else
				{
					u8 r = *((u8*)image.GetDataPtr() + channelStride * i + 0);
					u8 g = 0;
					if (channelStride >= 2)
						g = *((u8*)image.GetDataPtr() + channelStride * i + 1);
					u8 b = 0;
					if (channelStride >= 3)
						b = *((u8*)image.GetDataPtr() + channelStride * i + 2);
					u8 a = 0;
					if (channelStride >= 4)
						a = *((u8*)image.GetDataPtr() + channelStride * i + 3);

					stream << r;
					if (numChannels >= 2)
						stream << g;
					if (numChannels >= 3)
						stream << b;
					if (numChannels >= 4)
						stream << a;
				}
			}

			texture->source.LoadData(stream.GetRawDataPtr(), stream.GetLength());
		}

		return true;
	}

} // namespace CE::Editor
