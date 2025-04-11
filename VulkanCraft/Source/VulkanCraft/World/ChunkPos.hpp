#pragma once

#include <Engine.hpp>
#include <xhash>

using namespace eng;

namespace vc
{
    using ChunkPos = ivec3;

    struct ChunkPosHash
    {
        ENG_NO_DISCARD u64 operator()(ChunkPos chunkPos) const noexcept;
    };
}
