#pragma once

namespace CE::RPI
{
	struct alignas(16) LightConstants
	{
		Vec4 ambientColor{};
		u32 totalDirectionalLights;
		u32 totalLocalLights;
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