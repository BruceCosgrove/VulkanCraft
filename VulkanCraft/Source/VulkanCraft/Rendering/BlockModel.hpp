#pragma once

#include "VulkanCraft/Rendering/TextureID.hpp"

namespace vc
{
    struct BlockModel
    {
        u8 SolidBits = 0;
        TextureID Left{};
        TextureID Right{};
        TextureID Bottom{};
        TextureID Top{};
        TextureID Back{};
        TextureID Front{};
    };
}
