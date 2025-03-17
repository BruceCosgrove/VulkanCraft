#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <imgui.h>

namespace vc
{
    class ImGuiHelper
    {
        ENG_STATIC_CLASS(ImGuiHelper);
    public:
        static void Initialize();
        static void Shutdown();
    };
}
