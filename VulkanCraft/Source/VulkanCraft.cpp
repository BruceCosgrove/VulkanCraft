#include <Engine/Entrypoint.hpp>
#include "VulkanCraftLayer.hpp"

eng::EngineInfo ProvideEngineInfo(int argc, char** argv)
{
    eng::EngineInfo engineInfo;
    auto& applicationInfo = engineInfo.ApplicationInfo;
    auto& logInfo = engineInfo.LogInfo;

    logInfo.LogFileName = "VulkanCraft.log";

    engineInfo.OnEngineInitialized = [](eng::Application& application)
    {
        auto& layerStack = application.GetLayerStack();

        layerStack.PushLayer(std::make_unique<vc::VulkanCraftLayer>());
    };

    return engineInfo;
}
