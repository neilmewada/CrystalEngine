
#include "PAL/Windows/WindowsProcess.h"
#include "PAL/Windows/WindowsDirectories.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace CE
{
	void WindowsProcess::Crash()
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}

	void* WindowsProcess::LoadDll(IO::Path path)
	{
		auto string = path.GetString();
		return LoadLibraryA(string.GetCString());
	}

	void* WindowsProcess::GetDllSymbol(void* dllHandle, const char* procName)
	{
		return GetProcAddress((HMODULE)dllHandle, procName);
	}

	void WindowsProcess::UnloadDll(void* dllHandle)
	{
		FreeLibrary((HMODULE)dllHandle);
	}

	IO::Path WindowsProcess::GetModuleDllPath(String moduleName)
	{
		String dllName = moduleName + ".dll";
		return PlatformDirectories::GetLaunchDir() / dllName;
	}

	String WindowsProcess::GetDllDecoratedName(String baseName)
	{
		return baseName + ".dll";
	}

    bool WindowsProcess::LaunchProcess(const IO::Path& executablePath, const String& args)
    {
		String command = "\"" + executablePath.GetString() + "\" " + args;

		STARTUPINFO startupInfo;
		PROCESS_INFORMATION processInfo;

		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		ZeroMemory(&processInfo, sizeof(processInfo));

		CreateProcessA(NULL, command.GetCString(), NULL, NULL, NULL,
			NULL, NULL, NULL, &startupInfo, &processInfo);
		
		return true;
    }

}
