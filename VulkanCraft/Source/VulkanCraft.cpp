#include "Engine/Entrypoint.hpp"

eng::EngineInfo ProvideEngineInfo(int argc, char** argv)
{
    eng::EngineInfo engineInfo;
    auto& applicationInfo = engineInfo.ApplicationInfo;
    auto& logInfo = engineInfo.LogInfo;

    logInfo.LogFileName = "VulkanCraft.log";

    return engineInfo;
}
