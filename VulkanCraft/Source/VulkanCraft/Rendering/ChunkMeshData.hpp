#pragma once

#include "VulkanCraft/World/ChunkPos.hpp"
#include <vector>

using namespace eng;

namespace vc
{
    struct ChunkMeshData
    {
        ChunkPos ChunkPos;
        std::vector<uvec2> Left;
        std::vector<uvec2> Right;
        std::vector<uvec2> Bottom;
        std::vector<uvec2> Top;
        std::vector<uvec2> Back;
        std::vector<uvec2> Front;
    };
}
