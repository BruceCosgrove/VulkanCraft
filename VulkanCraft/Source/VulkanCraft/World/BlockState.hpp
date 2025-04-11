#pragma once

#include "VulkanCraft/World/Block.hpp"

namespace vc
{
    enum class BlockStateID : u32 {};

    struct BlockState
    {
        BlockID BlockID{};
        // TODO: variants
    };
}
