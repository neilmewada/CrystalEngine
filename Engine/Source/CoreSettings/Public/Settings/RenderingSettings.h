#pragma once

namespace CE
{
	// This class is only for an example!

    CLASS(Settings = "RenderingSettings", DisplayName = "Rendering", SettingsCategory = "Project")
	class CORESETTINGS_API RenderingSettings : public Settings
	{
		CE_CLASS(RenderingSettings, Settings)
	public:

		virtual ~RenderingSettings();

		FIELD(EditAnywhere, Category = "Ray Tracing")
		bool enableRayTracing = true;

	};

} // namespace CE

#include "RenderingSettings.rtti.h"
