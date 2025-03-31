#pragma once

#include <Engine.hpp>
#include <xhash>

using namespace eng;

namespace vc
{
    using ChunkPos = ivec3;

    struct ChunkPosHash
    {
        ENG_NO_DISCARD u64 operator()(ChunkPos chunkPos) const noexcept
        {
            auto hasher = std::hash<i32>();
            return hasher(chunkPos.x) << 32 ^ hasher(chunkPos.y) << 16 ^ hasher(chunkPos.z);
        }
    };
}
