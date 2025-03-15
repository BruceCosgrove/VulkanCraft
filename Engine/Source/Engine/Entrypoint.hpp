#pragma once

#include "Engine/Core/Application.hpp"
#include "Engine/Core/Log.hpp"

namespace eng
{
    struct EngineInfo
    {
        ApplicationInfo ApplicationInfo;
        LogInfo LogInfo;
    };
}

// To be defined in Client.
extern eng::EngineInfo ProvideEngineInfo(int argc, char** argv);
