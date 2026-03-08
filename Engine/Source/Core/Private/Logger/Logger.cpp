
#include "CoreTypes.h"
#include "Containers/Array.h"

#include "Logger/Logger.h"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#if PLATFORM_WINDOWS
#include "spdlog/sinks/wincolor_sink.h"
#endif
#include "spdlog/sinks/basic_file_sink.h"

#include "PAL/Common/PlatformDirectories.h"


namespace CE
{
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> GSystemConsoleSink;
    static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> GLogFileSink;

    static std::shared_ptr<spdlog::logger> GConsoleLogger;
    static std::shared_ptr<spdlog::logger> GFileLogger;

    static Array<spdlog::logger*> EditorLoggers{};

    static bool GIsLoggerInitialized = false;
    static CE::LogLevel GConsoleLogLevel = CE::LogLevel::Trace;
    static CE::LogLevel GFileDumpLogLevel = CE::LogLevel::Trace;

    void Logger::SetConsoleLogLevel(LogLevel level)
    {
        GConsoleLogLevel = level;

        if (GIsLoggerInitialized)
        {
            GSystemConsoleSink->set_level((spdlog::level::level_enum)GConsoleLogLevel);
        }
    }

    void Logger::SetFileDumpLogLevel(LogLevel level)
    {
        GFileDumpLogLevel = level;

        if (GIsLoggerInitialized)
        {
            GLogFileSink->set_level((spdlog::level::level_enum)GFileDumpLogLevel);
        }
    }

    void Logger::Initialize()
    {
        spdlog::init_thread_pool(8192, 1);

        GSystemConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        GSystemConsoleSink->set_level((spdlog::level::level_enum)GConsoleLogLevel);
        GSystemConsoleSink->set_pattern("[%^%l%$] %v");

        String logName = "Log";
        if (gProjectName.NotEmpty())
        {
            logName = gProjectName;
        }

        auto logPath = (PlatformDirectories::GetLogDir() / (logName + " " + DateTime::Now().ToString().Replace({':'}, '-') + ".txt")).GetString();

        GLogFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.GetCString());
        GLogFileSink->set_level((spdlog::level::level_enum)GFileDumpLogLevel);
        GLogFileSink->set_pattern("[%Y-%m-%d %T] [%l] %v");

        std::vector<spdlog::sink_ptr> consoleSinks{ GSystemConsoleSink };
        
        GConsoleLogger = std::make_shared<spdlog::logger>("ConsoleLog", consoleSinks.begin(), consoleSinks.end());
        GFileLogger = std::make_shared<spdlog::logger>("FileLog", GLogFileSink);
        
        GIsLoggerInitialized = true;
    }

    void Logger::Shutdown()
    {
        GSystemConsoleSink.reset();
        GLogFileSink.reset();
        GConsoleLogger.reset();
        GFileLogger.reset();

        GIsLoggerInitialized = false;
    }

    void Logger::Log(LogLevel level, StringView message, LogTarget target)
    {
        if (!GIsLoggerInitialized)
            return;

        if (level == LogLevel::Crash)
            level = LogLevel::Critical;

        if (EnumHasAnyFlags(target, LogTarget::Console))
        {
            GConsoleLogger->log((spdlog::level::level_enum)level, message.GetCString());

            if (EditorLoggers.GetSize() > 0)
            {
                for (auto editorLogger : EditorLoggers)
                {
                    editorLogger->log((spdlog::level::level_enum)level, message.GetCString());
                }
            }
        }

        if (EnumHasAnyFlags(target, LogTarget::LogFile))
        {
            GFileLogger->log((spdlog::level::level_enum)level, message.GetCString());
        }
    }

    void Logger::Log(LogLevel level, StringView message, const char* fileName, int line, LogTarget target)
    {
        if (!GIsLoggerInitialized)
            return;

#if !CE_BUILD_RELEASE // For Non-release builds
        if (level == LogLevel::Crash)
        {
            String fullMessage = String::Format("{}\n{}", message.GetCString(), cpptrace::generate_trace().to_string(true));
            Log(level, fullMessage, target);
            Flush();
            PlatformProcess::Crash();
        }
        else
        {
            Log(level, message, target);
        }
#else
        Log(level, message, target);
#endif
    }

    void Logger::Flush()
    {
        if (GConsoleLogger)
			GConsoleLogger->flush();

        if (EditorLoggers.GetSize() > 0)
        {
            for (auto editorLogger : EditorLoggers)
            {
                editorLogger->flush();
            }
        }
    }

    std::shared_ptr<spdlog::logger> Logger::GetConsoleLogger()
    {
        return GConsoleLogger;
    }

    void Logger::AddEditorLogger(spdlog::logger* logger)
    {
        EditorLoggers.Add(logger);
    }

    void Logger::RemoveEditorLogger(spdlog::logger* logger)
    {
        EditorLoggers.Add(logger);
    }

} // namespace CE
