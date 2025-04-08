#pragma once

#include "VulkanCraft/World/Chunk.hpp"
#include <Engine.hpp>
#include <memory>
#include <unordered_map>

using namespace eng;

namespace vc
{
    class World
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(World);
    public:
        World();
    public:
        Chunk* GetChunk(ChunkPos chunkPos);
    private:
        friend class WorldRenderer;
        BlockRegistry m_BlockRegistry;
        std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> m_Chunks;
    };
}
