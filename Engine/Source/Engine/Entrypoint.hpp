#pragma once

#include "Engine/Core/Application.hpp"
#include "Engine/Logging/Log.hpp"

namespace eng
{
    struct EngineInfo
    {
        ApplicationInfo ApplicationInfo;
        LogInfo LogInfo;
    };
}

// To be defined in Client.
extern eng::EngineInfo GetEngineInfo(int argc, char** argv);
