#include "Chunk.hpp"
#include "VulkanCraft/Rendering/BlockModel.hpp"
#include "VulkanCraft/Rendering/ChunkMeshData.hpp"
#include "VulkanCraft/World/BlockRegistry.hpp"
#include "VulkanCraft/World/World.hpp"

namespace vc
{
    Chunk::Chunk(World& world, BlockRegistry& blockRegistry, ChunkPos position)
        : m_World(world)
        , m_BlockRegistry(blockRegistry)
        , m_Position(position)
    {

    }

    Chunk::~Chunk()
    {

    }

    ChunkPos Chunk::GetPosition() const
    {
        return m_Position;
    }

    void Chunk::GenerateTerrain()
    {
        if (m_Generating.exchange(true, std::memory_order_relaxed))
            return;
        Application::Get().ExecuteAsync([this, c = shared_from_this()]
        {
            Timer timer("Chunk::GenerateTerrain");

            BlockID air = m_BlockRegistry.GetBlock("minecraft:air");
            BlockID bedrock = m_BlockRegistry.GetBlock("minecraft:bedrock");
            BlockID stone = m_BlockRegistry.GetBlock("minecraft:stone");
            BlockID dirt = m_BlockRegistry.GetBlock("minecraft:dirt");
            BlockID grass = m_BlockRegistry.GetBlock("minecraft:grass");

            // TODO: terrain generation
            // IMPORTANT: always create blockstates in z->y->x index order.
            for (u8 z = 0; z < Size; z++)
            {
                for (u8 y = 0; y < Size; y++)
                {
                    for (u8 x = 0; x < Size; x++)
                    {
                        u8 xyz = x + y + z;
                        BlockID blockID = air;
                        if ((xyz & 1) == 0) blockID = BlockID(((xyz >> 1) & 3) + 2);
                        //if (y == 0) blockID = bedrock;
                        //else if (y < 7) blockID = stone;
                        //else if (y < 10) blockID = dirt;
                        //else if (y == 10 and (x % Size != u32(m_Position.x) % Size or z % Size != u32(m_Position.z) % Size)) blockID = grass;
                        m_BlockStateRegistry.CreateBlockState(blockID);
                    }
                }
            }

            m_GenerationStage.store(ChunkGenerationStage::Terrain, std::memory_order_release);
            m_Generating.store(false, std::memory_order_relaxed);
        });
    }

    void Chunk::GenerateMesh()
    {
        if (m_Generating.exchange(true, std::memory_order_relaxed))
            return;
        Application::Get().ExecuteAsync([this, c = shared_from_this()]
        {
            Timer timer("Chunk::GenerateMesh");

            // TODO: this seems potentially suited for gpu compute.

            Chunk* leftChunk   = m_World.GetChunk({m_Position.x - 1, m_Position.y, m_Position.z});
            Chunk* rightChunk  = m_World.GetChunk({m_Position.x + 1, m_Position.y, m_Position.z});
            Chunk* bottomChunk = m_World.GetChunk({m_Position.x, m_Position.y - 1, m_Position.z});
            Chunk* topChunk    = m_World.GetChunk({m_Position.x, m_Position.y + 1, m_Position.z});
            Chunk* backChunk   = m_World.GetChunk({m_Position.x, m_Position.y, m_Position.z - 1});
            Chunk* frontChunk  = m_World.GetChunk({m_Position.x, m_Position.y, m_Position.z + 1});

            std::vector<u32> faceSolidMasks(Size2 * 6);

            // Represent the solid state of block faces in binary.
            auto blockStates = m_BlockStateRegistry.GetView<BlockState>(); // NOTE: These block states will be used later too.
            for (auto e : blockStates)
            {
                // TODO: variants
                auto& blockState = blockStates.get<BlockState>(e);
                // Only sample block states with a model.
                if (auto* model = m_BlockRegistry.TryGetComponent<BlockModel>(blockState.BlockID))
                {
                    // Use the entt::entity/id_type as the block index.
                    u32 index = std::to_underlying(e);
                    u8 x = index % Size;
                    u8 y = index / Size % Size;
                    u8 z = index / Size2;

                    auto constructMask = [&faceSolidMasks, &model = *model](u8 face, u8 px, u8 py, u8 pz)
                    {
                        faceSolidMasks[px + Size * py + Size2 * face] |= (model.SolidBits >> (face ^ 1) & 1) << pz;
                    };

                    constructMask(0, z, y, x + 1);  // left
                    constructMask(1, z, y, 16 - x); // right
                    constructMask(2, x, z, y + 1);  // bottom
                    constructMask(3, x, z, 16 - y); // top
                    constructMask(4, x, y, z + 1);  // back
                    constructMask(5, x, y, 16 - z); // front
                }
            }

            ChunkMeshData meshData;

            // If the whole chunk has no solid faces, there is no mesh.
            // NOTE: this checks if faceSolidMasks is all zeros.
            if (faceSolidMasks.front() != 0 or
                std::memcmp(faceSolidMasks.data(), faceSolidMasks.data() + 1, faceSolidMasks.size() - 1) != 0)
            {
                // TODO: the order { front back bottom left right top } seems optimal for cache (profile).

                auto insertChunkEdge = [this, &faceSolidMasks](Chunk* chunk, u8 face, u8 x, u8 y, u8 z, u8 px, u8 py)
                {
                    auto& mask = faceSolidMasks[px + Size * py + Size2 * face];
                    if (chunk)
                    {
                        auto blockStates = chunk->m_BlockStateRegistry.GetView<BlockState>();
                        auto e = static_cast<entt::entity>(x + Size * (y + Size * z));
                        auto& blockState = blockStates.get<BlockState>(e);
                        if (auto* model = m_BlockRegistry.TryGetComponent<BlockModel>(blockState.BlockID))
                            mask |= model->SolidBits >> (face ^ 1) & 1;
                    }
                    // Treat non-existent chunks as solid.
                    else
                        mask |= 1;
                };

                for (u8 py = 0; py < Size; py++)
                {
                    for (u8 px = 0; px < Size; px++)
                    {
                        insertChunkEdge(leftChunk,   0, 15, py, px, px, py);
                        insertChunkEdge(rightChunk,  1, 0,  py, px, px, py);
                        insertChunkEdge(bottomChunk, 2, px, 15, py, px, py);
                        insertChunkEdge(topChunk,    3, px, 0,  py, px, py);
                        insertChunkEdge(backChunk,   4, px, py, 15, px, py);
                        insertChunkEdge(frontChunk,  5, px, py, 0,  px, py);
                    }
                }

                // Face culling. Yes, it's this simple.
                for (auto& mask : faceSolidMasks)
                    mask = u16((mask & ~(mask << 1)) >> 1);

                // Populate greedy meshing planes.
                // TODO: variants, e.g. rotations, will be added to the hashed key type.
                // NOTE: These are stored using std::array since they will already be on the heap from std::unordered_map.
                std::unordered_map<TextureID, std::array<u16, Size2 * 6>> greedyMeshingPlanes;

                u16 maskIndex = 0;
                for (u8 face = 0; face < 6; face++)
                {
                    for (u8 py = 0; py < Size; py++)
                    {
                        for (u8 px = 0; px < Size; px++)
                        {
                            // Get the current mask.
                            u16 mask = faceSolidMasks[maskIndex++];
                            while (mask)
                            {
                                // Calculate the block plane z coord.
                                u8 pz = std::countr_zero(mask);
                                // Mark this bit as completed (unset it).
                                mask &= mask - 1;

                                // Get the true local block position.
                                auto planeToLocal = std::to_array({
                                    u16(u16(pz     ) | u16(py     ) << 4 | u16(px     ) << 8), // left
                                    u16(u16(15 - pz) | u16(py     ) << 4 | u16(px     ) << 8), // right
                                    u16(u16(px     ) | u16(pz     ) << 4 | u16(py     ) << 8), // bottom
                                    u16(u16(px     ) | u16(15 - pz) << 4 | u16(py     ) << 8), // top
                                    u16(u16(px     ) | u16(py     ) << 4 | u16(pz     ) << 8), // back
                                    u16(u16(px     ) | u16(py     ) << 4 | u16(15 - pz) << 8), // front
                                });
                                u16 index = planeToLocal[face];

                                auto e = static_cast<entt::entity>(index);
                                auto& blockState = blockStates.get<BlockState>(e);
                                // NOTE: No need to try get again, it was already checked and does exist.
                                // TODO: How to not sample twice? Can't exactly cache the texture ids since
                                // that would just move the problem and add overhead.
                                auto& model = m_BlockRegistry.GetComponent<BlockModel>(blockState.BlockID);
                                // Put this face in the appropriate greedy meshing plane.
                                TextureID textureID = (&model.Left)[face];
                                greedyMeshingPlanes[textureID][py + Size * pz + Size2 * face] |= u16(1 << px);
                            }
                        }
                    }
                }

                // Greedy mesh each plane.
                struct GreedyQuad
                {
                    u8 x, y, z, w, h;
                };

                //                   packedFaceData.y                 packedFaceData.x
                // 64  60  56  52  48  44  40  36  32   28  24  20  16  12   8   4   0
                //   ----------------------------hhhh wwwwzzzzyyyyxxxxtttttttttttttttt
                // t = texture id, x = local x, y = local y, z = local z, w = width, h = height
                auto packVertex = [](TextureID textureID, GreedyQuad quad) -> uvec2
                {
                    return
                    {
                        (quad.w - 1) << 28 | quad.z << 24 | quad.y << 20 | quad.x << 16 | std::to_underlying(textureID),
                        /* extra 24 bits */ (quad.h - 1),
                    };
                };

                for (auto& [textureID, facePlanes] : greedyMeshingPlanes)
                {
                    for (u8 face = 0; face < 6; face++)
                    {
                        auto& packedVertices = (&meshData.Left)[face];
                        for (u8 pz = 0; pz < Size; pz++)
                        {
                            auto plane = std::span(facePlanes).subspan(Size * pz + Size2 * face, Size);

                            for (u8 py = 0; py < Size; py++)
                            {
                                for (u8 px = 0; px < Size; )
                                {
                                    // Expand the quad horizontally.
                                    px += std::countr_zero<u16>(plane[py] >> px);
                                    // If there's no solid faces left, stop advancing.
                                    if (px >= Size)
                                        break;

                                    u8 width = std::countr_one<u16>(plane[py] >> px);
                                    u16 widthMask = ((1 << width) - 1) << px;
                                    // Consume this mask.
                                    plane[py] &= ~widthMask;

                                    // Expand the quad vertically.
                                    u8 height = 1;
                                    while (py + height < Size)
                                    {
                                        u16 nextMask = plane[py + height] & widthMask;
                                        // Stop expanding if the next mask doesn't have all
                                        // solid faces set as the initial width mask.
                                        if (nextMask != widthMask)
                                            break;
                                        // Expand into the next mask.
                                        plane[py + height] &= ~widthMask;
                                        height++;
                                    }

                                    // Convert plane coords to chunk coords.
                                    // TODO: try to not duplicate this work.
                                    auto planeToLocal = std::to_array({
                                        u16(u16(pz     ) | u16(py     ) << 4 | u16(px     ) << 8), // left
                                        u16(u16(15 - pz) | u16(py     ) << 4 | u16(px     ) << 8), // right
                                        u16(u16(px     ) | u16(pz     ) << 4 | u16(py     ) << 8), // bottom
                                        u16(u16(px     ) | u16(15 - pz) << 4 | u16(py     ) << 8), // top
                                        u16(u16(px     ) | u16(py     ) << 4 | u16(pz     ) << 8), // back
                                        u16(u16(px     ) | u16(py     ) << 4 | u16(15 - pz) << 8), // front
                                    });
                                    u16 index = planeToLocal[face];
                                    u8 x = index % Size;
                                    u8 y = index / Size % Size;
                                    u8 z = index / Size2;

                                    // Pack the quad into the appropriate submesh.
                                    packedVertices.push_back(packVertex(textureID, {x, y, z, width, height}));

                                    px += width;
                                }
                            }
                        }
                    }
                }
            }

            m_StageOutput = meshData;
            m_GenerationStage.store(ChunkGenerationStage::Mesh, std::memory_order_release);
            m_Generating.store(false, std::memory_order_relaxed);
        });
    }

    ChunkGenerationStage Chunk::GetGenerationStage() const
    {
        return m_GenerationStage.load(std::memory_order_acquire);
    }

    bool Chunk::IsGenerating() const
    {
        return m_Generating.load(std::memory_order_relaxed);
    }
}
