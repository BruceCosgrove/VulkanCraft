#pragma once

#include "VulkanCraft/Rendering/TextureID.hpp"

namespace vc
{
    struct BlockModel
    {
        TextureID Left{};
        TextureID Right{};
        TextureID Bottom{};
        TextureID Top{};
        TextureID Back{};
        TextureID Front{};
    };
}
