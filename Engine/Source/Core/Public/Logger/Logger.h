#pragma once

#include "LogMacros.h"
#include "Misc/CoreDefines.h"
#include "Misc/EnumClass.h"

namespace spdlog
{
    class logger;
}

namespace CE
{
    class StringView;
    class String;

	enum class LogLevel
	{
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
        Off = 6,
        Crash = 7,
	};

    enum class LogTarget
    {
        None = 0,
        Console = BIT(0),
        LogFile = BIT(1),
        All = Console | LogFile,
    };

    ENUM_CLASS_FLAGS(LogTarget);

    class CORE_API Logger
    {
    public:
        CE_STATIC_CLASS(Logger);

        static void Initialize();
        static void Shutdown();

        static void SetConsoleLogLevel(LogLevel level);
        static void SetFileDumpLogLevel(LogLevel level);

        static void Log(LogLevel level, StringView message, LogTarget target = LogTarget::All);
        static void Log(LogLevel level, StringView message, const char* fileName, int line, LogTarget target = LogTarget::All);

        static void Flush();

        static std::shared_ptr<spdlog::logger> GetConsoleLogger();

        static void AddEditorLogger(spdlog::logger* logger);
        static void RemoveEditorLogger(spdlog::logger* logger);
        
    private:
        
        
    };

} // namespace CE
