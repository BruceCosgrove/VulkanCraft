#pragma once

#include "Engine/Core/Application.hpp"
#include "Engine/Core/Log.hpp"

namespace eng
{
    using EngineInitializedCallback = void(*)(Application&);

    struct EngineInfo
    {
        ApplicationInfo ApplicationInfo;
        LogInfo LogInfo;
        EngineInitializedCallback OnEngineInitialized = nullptr;
    };
}

// To be defined in Client.
extern eng::EngineInfo ProvideEngineInfo(int argc, char** argv);
