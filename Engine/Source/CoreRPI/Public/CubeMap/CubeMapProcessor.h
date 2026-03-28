#pragma once

namespace CE::RPI
{
    
	struct CubeMapOfflineProcessInfo
	{
		Name name{};
		CAImage sourceImage{};
		RPI::Shader* equirectangularShader = nullptr;
		RPI::Shader* mipMapShader = nullptr;

		// Diffuse IBL shaders
		RPI::Shader* grayscaleShader = nullptr;
		RPI::Shader* rowAverageShader = nullptr;
		RPI::Shader* columnAverageShader = nullptr;
		RPI::Shader* divisionShader = nullptr;
		RPI::Shader* cdfConditionalInverseShader = nullptr;
		RPI::Shader* cdfMarginalInverseShader = nullptr;
		RPI::Shader* diffuseConvolutionShader = nullptr;

		RPI::Shader* specularConvolutionShader = nullptr;

		bool useCompression = false;
		bool compressDiffuseIrradiance = false;

		// Use 0 to auto choose resolution based on source image
		u32 cubeMapResolution = 0;

		// Set to 0 to disable diffulse IBL irradiance computation
		u32 diffuseIrradianceResolution = 32;

		// If we should perform specular IBL convolution
		bool specularConvolution = false;

		BinaryBlob* diffuseIrradianceOutput = nullptr;
	};

	class CORERPI_API CubeMapProcessor
	{
	public:

		CubeMapProcessor() = default;
		virtual ~CubeMapProcessor() = default;

		// Takes an equirectangular HDRI raw image (in CPU memory) and generates a cubemap out of it, and optionally generate IBL maps.
		bool ProcessCubeMapOffline(const CubeMapOfflineProcessInfo& processInfo, BinaryBlob& cubeMapOutput, u32& outMipLevels);

	private:

	};

} // namespace CE::RPI
