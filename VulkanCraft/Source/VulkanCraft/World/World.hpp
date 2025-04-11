#pragma once

#include "VulkanCraft/World/Chunk.hpp"
#include "VulkanCraft/World/ChunkGenerator.hpp"
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
        World(BlockRegistry const& blocks);

        void OnUpdate(Timestep timestep, ChunkGenerator& chunkGenerator);

        Chunk* GetChunk(ChunkPos chunkPos);
    private:
        friend class WorldRenderer;
        BlockRegistry const& m_Blocks; // non-owning
        std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> m_Chunks;
    };
}
