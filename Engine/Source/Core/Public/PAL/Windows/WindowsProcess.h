#pragma once

#if PLATFORM_WINDOWS

#include "CoreTypes.h"
#include "IO/Path.h"

namespace CE
{
    
    class CORE_API WindowsProcess
    {
    public:
        CE_STATIC_CLASS(WindowsProcess);

        static void Crash();

        static void* LoadDll(IO::Path path);

        static void* GetDllSymbol(void* dllHandle, const char* procName);

        static void UnloadDll(void* dllHandle);

        static IO::Path GetModuleDllPath(String moduleName);

        static String GetDllDecoratedName(String baseName);

		static bool LaunchProcess(const IO::Path& executablePath, const String& args);
    };

    typedef WindowsProcess PlatformProcess;

} // namespace CE


#endif
