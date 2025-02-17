#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace eng
{
    struct LogInfo
    {
        spdlog::level::level_enum EngineLoggingLevel = spdlog::level::trace;
        spdlog::level::level_enum ClientLoggingLevel = spdlog::level::trace;
        std::string LogFileName = "Engine.log";
    };

    class Log
    {
    public:
        static spdlog::logger& GetEngineLogger();
        static spdlog::logger& GetClientLogger();
    private:
        inline static std::shared_ptr<spdlog::logger> s_EngineLogger;
        inline static std::shared_ptr<spdlog::logger> s_ClientLogger;
    private:
        friend int Main(int argc, char** argv);
        static void Initialize(const LogInfo& info);
        static void Shutdown();
    };
}

#if ENG_ENGINE
    #define ENG_LOG_TRACE(format, ...) ::eng::Log::GetEngineLogger().trace(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_DEBUG(format, ...) ::eng::Log::GetEngineLogger().debug(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_INFO(format, ...)  ::eng::Log::GetEngineLogger().info(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_WARN(format, ...)  ::eng::Log::GetEngineLogger().warn(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_ERROR(format, ...) ::eng::Log::GetEngineLogger().error(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_FATAL(format, ...) ::eng::Log::GetEngineLogger().critical(format __VA_OPT__(,) __VA_ARGS__)
#else
    #define ENG_LOG_TRACE(format, ...) ::eng::Log::GetClientLogger().trace(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_DEBUG(format, ...) ::eng::Log::GetClientLogger().debug(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_INFO(format, ...)  ::eng::Log::GetClientLogger().info(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_WARN(format, ...)  ::eng::Log::GetClientLogger().warn(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_ERROR(format, ...) ::eng::Log::GetClientLogger().error(format __VA_OPT__(,) __VA_ARGS__)
    #define ENG_LOG_FATAL(format, ...) ::eng::Log::GetClientLogger().critical(format __VA_OPT__(,) __VA_ARGS__)
#endif
