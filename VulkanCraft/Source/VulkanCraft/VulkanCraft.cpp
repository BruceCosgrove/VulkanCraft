#include <Engine/Entrypoint.hpp>
#include "VulkanCraftLayer.hpp"

eng::EngineInfo ProvideEngineInfo(int argc, char** argv)
{
    eng::EngineInfo engineInfo;

    auto& applicationInfo = engineInfo.ApplicationInfo;

    auto& windowInfo = applicationInfo.WindowInfo;
    windowInfo.Title = "VulkanCraft";
    windowInfo.Layers.Add<vc::VulkanCraftLayer>();

    auto& logInfo = engineInfo.LogInfo;
    logInfo.LogFileName = "VulkanCraft.log";

    return engineInfo;
}
