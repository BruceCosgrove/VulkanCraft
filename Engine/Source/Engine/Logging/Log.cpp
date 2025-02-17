#include "Log.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#if ENG_ENABLE_CONSOLE
    #include <spdlog/sinks/stdout_color_sinks.h>
#endif
#include <vector>

namespace eng
{
    spdlog::logger& Log::GetEngineLogger()
    {
        return *s_EngineLogger;
    }

    spdlog::logger& Log::GetClientLogger()
    {
        return *s_ClientLogger;
    }

    void Log::Initialize(const LogInfo& info)
    {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(info.LogFileName, true))->set_pattern("[%T] [%l] %n: %v");
#if ENG_ENABLE_CONSOLE
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>())->set_pattern("%^[%T] %n: %v%$");
#endif

        s_EngineLogger = std::make_shared<spdlog::logger>("Engine", sinks.begin(), sinks.end());
        spdlog::register_logger(s_EngineLogger);
        s_EngineLogger->set_level(info.EngineLoggingLevel);
        s_EngineLogger->flush_on(info.EngineLoggingLevel);

        s_ClientLogger = std::make_shared<spdlog::logger>("Client", sinks.begin(), sinks.end());
        spdlog::register_logger(s_ClientLogger);
        s_ClientLogger->set_level(info.ClientLoggingLevel);
        s_ClientLogger->flush_on(info.ClientLoggingLevel);
    }

    void Log::Shutdown()
    {
        s_EngineLogger.reset();
        s_ClientLogger.reset();
        spdlog::shutdown();
    }
}
