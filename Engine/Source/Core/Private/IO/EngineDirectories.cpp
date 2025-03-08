#include "Core.h"

namespace CE
{

    IO::Path EngineDirectories::GetEngineInstallDirectory()
    {
        return PlatformDirectories::GetLaunchDir();
    }

    IO::Path EngineDirectories::GetGameInstallDirectory()
    {
        return PlatformDirectories::GetLaunchDir();
    }

    IO::Path EngineDirectories::GetGameDirectory()
    {
#if PAL_TRAIT_BUILD_EDITOR
        ASSERT(!gProjectPath.IsEmpty(), "GetGameDir() called while project path is NOT set!");
        return gProjectPath / "Game";
#else
        return GetGameInstallDirectory() / "Game";
#endif
    }
} // namespace CE
