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
            auto chunksToLoad = std::to_array<ChunkGenerator::QueuedChunk>({
                {{-2, 0, -2}, true},
                {{-1, 0, -2}, true},
                {{0, 0, -2}, true},
                {{1, 0, -2}, true},

                {{-2, 0, -1}, true},
                {{-1, 0, -1}, true},
                {{0, 0, -1}, true},
                {{1, 0, -1}, true},

                {{-2, 0, 0}, true},
                {{-1, 0, 0}, true},
                {{0, 0, 0}, true},
                {{1, 0, 0}, true},

                {{-2, 0, 1}, true},
                {{-1, 0, 1}, true},
                {{0, 0, 1}, true},
                {{1, 0, 1}, true},
            });
            chunkGenerator.QueueChunks(chunksToLoad);
        }

        chunkGenerator.ConsumeGeneratedChunks([this](std::shared_ptr<Chunk>&& chunk)
        {
            m_Chunks[chunk->GetPosition()] = std::move(chunk);
        });
    }

    Chunk* World::GetChunk(ChunkPos chunkPos)
    {
        if (auto it = m_Chunks.find(chunkPos); it != m_Chunks.end())
            return it->second.get();
        return nullptr;
    }
}
