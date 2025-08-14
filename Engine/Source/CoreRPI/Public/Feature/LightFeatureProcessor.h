#pragma once

namespace CE::RPI
{
	struct alignas(16) LightConstants
	{
		Vec4 ambientColor{};
		u32 totalDirectionalLights = 0;
		u32 totalLocalLights = 0;
		u32 tileSizeX = 16;
		u32 tileSizeY = 16;
		u32 lightsPerTile = 0;
		u32 lightIndexPoolCapacity = 0;
	};

	CLASS(Abstract)
	class CORERPI_API LightFeatureProcessor : public FeatureProcessor
	{
		CE_CLASS(LightFeatureProcessor, FeatureProcessor)
	public:

		LightFeatureProcessor();

		virtual ~LightFeatureProcessor();

		void Render(const RenderPacket& packet) override;

	protected:

	};

} // namespace CE::RPI

#include "LightFeatureProcessor.rtti.h"