#include "ChunkPos.hpp"

namespace vc
{
    u64 ChunkPosHash::operator()(ChunkPos chunkPos) const noexcept
    {
        std::hash<i32> hasher;
        u64 hashX = hasher(chunkPos.x);
        u64 hashY = hasher(chunkPos.y);
        u64 hashZ = hasher(chunkPos.z);
        return (hashX << 32 | hashX >> 32) ^ (hashY << 16 | hashY >> 48) ^ hashZ;
    }
}
