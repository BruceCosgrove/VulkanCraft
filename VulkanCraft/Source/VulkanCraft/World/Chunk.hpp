#pragma once

#include "VulkanCraft/World/BlockRegistry.hpp"
#include "VulkanCraft/World/BlockStateRegistry.hpp"
#include "VulkanCraft/World/ChunkPos.hpp"
#include <Engine.hpp>

using namespace eng;

namespace vc
{
    class WorldRenderer;

    class Chunk : public std::enable_shared_from_this<Chunk>
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Chunk);
    public:
        // TODO: refactor constants
        inline static constexpr u32 Size = 16;
        inline static constexpr u32 Size2 = Size * Size;
        inline static constexpr u32 Size3 = Size * Size * Size;
    public:
        Chunk(BlockRegistry const& blocks, BlockStateRegistry&& blockStates, ChunkPos position);
        ~Chunk();

        ChunkPos GetPosition() const;
    private:
        BlockRegistry const& m_Blocks; // non-owning
        BlockStateRegistry m_BlockStates;
        ChunkPos m_Position;
    };
}
