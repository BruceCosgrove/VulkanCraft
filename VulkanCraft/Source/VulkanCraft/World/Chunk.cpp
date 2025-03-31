#include "Chunk.hpp"
#include "VulkanCraft/World/BlockRegistry.hpp"

namespace vc
{
    Chunk::Chunk(ivec3 position, RenderContext& context)
        : m_Position(position)
        , m_Context(context)
    {

    }

    Chunk::~Chunk()
    {

    }

    void Chunk::GenerateTerrain()
    {
        Application::Get().ExecuteAsync([c = shared_from_this()]
        {
            Chunk& chunk = *c;

            for (u8 z = 0; z < Size; z++)
            {
                for (u8 y = 0; y < Size; y++)
                {
                    for (u8 x = 0; x < Size; x++)
                    {
                        // TODO: BlockPos, EntityPos, ChunkPos
                        glm::i64vec3 blockPos = chunk.m_Position;
                        blockPos *= Size;
                        blockPos += ivec3(x, y, z);

                        BlockID blockID{1}; // air
                        if (y == 0) blockID = BlockID(2); // bedrock
                        else if (y < 7) blockID = BlockID(3); // stone
                        else if (y < 10) blockID = BlockID(4); // dirt
                        else if (y == 11) blockID = BlockID(5); // grass
                        chunk.m_BlockStates[Index(x, y, z)] = chunk.m_BlockStateRegistry.CreateBlockState(blockID);
                    }
                }
            }

            // TODO: how to propagate incremental changes.
            // terrain generation stages ... mesh generation
            chunk.GenerateMesh();
        });
    }

    void Chunk::GenerateMesh()
    {
        Application::Get().ExecuteAsync([c = shared_from_this()]() mutable
        {
            c->m_Mesh.Load([c = std::move(c)]
            {
                Chunk& chunk = *c;

                ChunkMeshInfo info;
                info.RenderContext = &chunk.m_Context;

                // TODO: here be the binary greedy mesher

                // TODO: this data comes from mesh generation
                info.Left   = {uvec2(0 << 16 | 3, 17 << 12 | 0)};
                info.Right  = {uvec2(1 << 16 | 3, 17 << 12 | 1 << 0)};
                info.Bottom = {uvec2(2 << 16 | 2, 17 << 12 | 0)};
                info.Top    = {uvec2(3 << 16 | 4, 17 << 12 | 1 << 4)};
                info.Back   = {uvec2(4 << 16 | 3, 17 << 12 | 0)};
                info.Front  = {uvec2(5 << 16 | 3, 17 << 12 | 1 << 8)};

                return std::make_shared<ChunkMesh>(info);
            });
        });
    }

    DynamicResource<std::shared_ptr<ChunkMesh>>& Chunk::GetMesh()
    {
        return m_Mesh;
    }

    u64 Chunk::Index(u8 x, u8 y, u8 z)
    {
        return x + (Size * (y + Size * u64(z)));
    }
}
