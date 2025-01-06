#pragma once

namespace CE::Editor
{
    
    CLASS(Config = Editor)
	class EDITORCORE_API TextureAssetImporter : public AssetImporter
	{
		CE_CLASS(TextureAssetImporter, AssetImporter)
	public:

		virtual Array<AssetImportJob*> CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets) override;

		Array<Name> GetProductAssetDependencies() override;

	private:

		FIELD(Config)
		TextureCompressionQuality compressionQuality = TextureCompressionQuality::Default;

		FIELD(Config)
		Array<String> ignoreCompressionFolders; // Folders where image compression is not applied

		FIELD(Config)
		u8 anisotropy = 0;

		FIELD(Config)
		bool importHdrAsCubemap = false;

		FIELD(Config)
		bool convoluteCubemap = false;

		FIELD(Config)
		u32 diffuseConvolutionResolution = 32;
		
		FIELD(Config)
		bool compressConvolution = false;

		FIELD(Config)
		bool specularConvolution = false;
	};

	class EDITORCORE_API TextureAssetImportJob : public AssetImportJob
	{
	public:
		typedef TextureAssetImportJob Self;
		typedef AssetImportJob Super;

		TextureAssetImportJob(AssetImporter* importer, const IO::Path& sourcePath, const IO::Path& outPath)
			: Super(importer, sourcePath, outPath)
		{

		}

		virtual bool ProcessAsset(const Ref<Bundle>& bundle) override;

	protected:

		bool ProcessTex2D(const String& name, Bundle* bundle, const CMImage& image, TextureFormat pixelFormat, 
			TextureSourceCompressionFormat compressionFormat,
			CMImageSourceFormat targetSourceFormat, Uuid textureUuid);

		bool ProcessCubeMap(const String& name, Bundle* bundle, const CMImage& sourceImage, TextureFormat pixelFormat,
			TextureSourceCompressionFormat compressionFormat,
			CMImageSourceFormat targetSourceFormat,
			Uuid cubeMapUuid, Uuid diffuseUuid);

	private:

		TextureCompressionQuality compressionQuality = TextureCompressionQuality::Default;
		Array<String> ignoreCompressionFolders; // Folders where image compression is not applied

		u8 anisotropy = 0;
		bool importHdrAsCubemap = false;
		bool convoluteCubemap = false;
		u32 diffuseConvolutionResolution = 32;
		bool compressConvolution = false;
		bool specularConvolution = false;

		friend class TextureAssetImporter;
	};

} // namespace CE::Editor

#include "TextureAssetImporter.rtti.h"