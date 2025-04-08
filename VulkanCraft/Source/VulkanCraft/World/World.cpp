#include "World.hpp"
#include "VulkanCraft/Rendering/BlockModel.hpp"

namespace vc
{
    World::World()
    {
        // TODO: block model files using yaml-cpp

        // air
        {
            m_BlockRegistry.CreateBlock("minecraft:air");
        }

        // bedrock
        {
            BlockID block = m_BlockRegistry.CreateBlock("minecraft:bedrock");
            BlockModel& model = m_BlockRegistry.EmplaceComponent<BlockModel>(block);
            model.SolidBits = 0b111111;
            model.Left = TextureID(0);
            model.Right = TextureID(0);
            model.Bottom = TextureID(0);
            model.Top = TextureID(0);
            model.Back = TextureID(0);
            model.Front = TextureID(0);
        }

        // stone
        {
            BlockID block = m_BlockRegistry.CreateBlock("minecraft:stone");
            BlockModel& model = m_BlockRegistry.EmplaceComponent<BlockModel>(block);
            model.SolidBits = 0b111111;
            model.Left = TextureID(1);
            model.Right = TextureID(1);
            model.Bottom = TextureID(1);
            model.Top = TextureID(1);
            model.Back = TextureID(1);
            model.Front = TextureID(1);
        }

        // dirt
        {
            BlockID block = m_BlockRegistry.CreateBlock("minecraft:dirt");
            BlockModel& model = m_BlockRegistry.EmplaceComponent<BlockModel>(block);
            model.SolidBits = 0b111111;
            model.Left = TextureID(2);
            model.Right = TextureID(2);
            model.Bottom = TextureID(2);
            model.Top = TextureID(2);
            model.Back = TextureID(2);
            model.Front = TextureID(2);
        }

        // grass
        {
            BlockID block = m_BlockRegistry.CreateBlock("minecraft:grass");
            BlockModel& model = m_BlockRegistry.EmplaceComponent<BlockModel>(block);
            model.SolidBits = 0b111111;
            model.Left = TextureID(3);
            model.Right = TextureID(3);
            model.Bottom = TextureID(2);
            model.Top = TextureID(4);
            model.Back = TextureID(3);
            model.Front = TextureID(3);
        }

        ivec3 min(-2, 0, -2);
        ivec3 max(1, 0, 1);
        for (i32 z = min.z; z <= max.z; z++)
        {
            for (i32 y = min.y; y <= max.y; y++)
            {
                for (i32 x = min.x; x <= max.x; x++)
                {
                    ivec3 position(x, y, z);
                    m_Chunks[position] = std::make_shared<Chunk>(*this, m_BlockRegistry, position);
                    m_Chunks[position]->GenerateTerrain();
                }
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
