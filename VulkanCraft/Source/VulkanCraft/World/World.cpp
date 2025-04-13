#include "World.hpp"
#include "VulkanCraft/Rendering/BlockModel.hpp"

namespace vc
{
    World::World(BlockRegistry const& blocks)
        : m_Blocks(blocks)
    {

    }

    void World::OnUpdate(Timestep timestep, ChunkGenerator& chunkGenerator)
    {
        if (static bool loaded = false; not loaded)
        {
            loaded = true;
            std::vector<ChunkPos> chunksToLoad;
            chunksToLoad.reserve(16 * 16 * 16);
            for (i32 z = -8; z < 8; z++)
                for (i32 y = -8; y < 8; y++)
                    for (i32 x = -8; x < 8; x++)
                        chunksToLoad.emplace_back(x, y, z);
            chunkGenerator.QueueChunkLoads(chunksToLoad);
        }

        chunkGenerator.ConsumeGeneratedChunks([this](std::shared_ptr<Chunk>&& chunk)
        {
            m_Chunks[chunk->GetPosition()] = std::move(chunk);
        });

        // TODO DEBUG TESTING:
        // removes a chunk every second.
        static float s_TotalTime = 0.0f;
        s_TotalTime += timestep;
        if (s_TotalTime >= 1.0f)
        {
            s_TotalTime -= 1.0f;
            if (not m_Chunks.empty())
            {
                auto it = m_Chunks.begin();
                ChunkPos chunkPos = it->second->GetPosition();
                m_Chunks.erase(it);
                chunkGenerator.QueueChunkUnloads(std::span{&chunkPos, 1});
            }
        }
    }

    Chunk* World::GetChunk(ChunkPos chunkPos)
    {
        if (auto it = m_Chunks.find(chunkPos); it != m_Chunks.end())
            return it->second.get();
        return nullptr;
    }
}
