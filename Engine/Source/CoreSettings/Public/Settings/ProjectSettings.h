#pragma once

namespace CE
{
    STRUCT()
    struct CORESETTINGS_API ProjectSettingsSigning final
    {
        CE_STRUCT(ProjectSettingsSigning)
    public:

        FIELD(EditAnywhere)
        String signingKey;

        FIELD(EditAnywhere)
        bool enabled = false;

    };

    STRUCT()
    struct CORESETTINGS_API ProjectSettingsAdvanced final
    {
        CE_STRUCT(ProjectSettingsAdvanced)
    public:

        FIELD(EditAnywhere)
        bool useBorderlessWindow = false;

        FIELD(EditAnywhere)
        bool useFullscreen = true;

        FIELD(EditAnywhere)
        bool allowMinimize = false;

        FIELD(EditAnywhere)
        ProjectSettingsSigning signing;

    };

    CLASS(Settings = "ProjectSettings", DisplayName = "Project", SettingsCategory = "Project")
    class CORESETTINGS_API ProjectSettings : public Settings
    {
        CE_CLASS(ProjectSettings, Settings)
    public:
        
        ProjectSettings() = default;

        // Description
        
        FIELD(EditAnywhere, Category = "Description", Validator = "Name")
		String projectName = "Untitled";

        FIELD(EditAnywhere, Category = "Description", Validator = "LongVersion")
		String projectVersion = "0.0.1";

        FIELD(EditAnywhere, Category = "Description")
		String description = "";

		// Legal

        FIELD(EditAnywhere, Category = "Legal")
		String copyrightNotice = "";

        FIELD(EditAnywhere, Category = "Legal", DisplayName = "Privacy Policy URL")
		String privacyPolicyUrl = "";

        // Advanced

        FIELD(EditAnywhere, Category = "Settings")
        ProjectSettingsAdvanced advanced;

    };
}

#include "ProjectSettings.rtti.h"