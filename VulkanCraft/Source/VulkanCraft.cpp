#include <Engine/Entrypoint.hpp>
#include "VulkanCraftLayer.hpp"

eng::EngineInfo ProvideEngineInfo(int argc, char** argv)
{
    eng::EngineInfo engineInfo;
    engineInfo.OnEngineInitialized = [](eng::Application& application)
    {
        auto& layerStack = application.GetWindow().GetLayerStack();
        layerStack.PushLayer(std::make_unique<vc::VulkanCraftLayer>());
    };

    auto& applicationInfo = engineInfo.ApplicationInfo;
    auto& windowInfo = applicationInfo.WindowInfo;
    windowInfo.Title = "VulkanCraft";

    auto& logInfo = engineInfo.LogInfo;
    logInfo.LogFileName = "VulkanCraft.log";

    return engineInfo;
}
