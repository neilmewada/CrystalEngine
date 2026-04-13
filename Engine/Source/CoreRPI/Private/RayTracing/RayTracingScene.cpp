#include "CoreRPI.h"

namespace CE::RPI
{
	RayTracingScene::RayTracingScene()
	{
		RHI::RayTracingTlasDescriptor tlasDesc{};
		tlas = RHI::gDynamicRHI->CreateRayTracingTlas(tlasDesc);
	}
} // namespace CE::RPI
