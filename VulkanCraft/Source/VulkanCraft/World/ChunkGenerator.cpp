#include "ChunkGenerator.hpp"
#include "VulkanCraft/World/Chunk.hpp"
#include "VulkanCraft/Rendering/BlockModel.hpp"

namespace vc
{
    // Chunk flow:
    //  QueueChunkLoads => m_QueuedChunkLoads => m_DelegatorThread
    //  QueueChunkUnloads => m_QueuedChunkUnloads => m_DelegatorThread
    //  QueueChunkRemeshes => m_QueuedChunkRemeshes => m_DelegatorThread
    //
    //  m_DelegatorThread => LoadChunk or UnloadChunk or RemeshChunk
    //      m_PendingChunks => m_GeneratableChunks
    //      m_UnloadingChunks => /dev/null
    //
    //  LoadChunk => m_PendingChunks or m_GeneratableChunks
    //  UnloadChunk => m_UnloadingChunks => m_DelegatorThread
    //  RemeshChunk => TODO
    //
    //  m_GeneratableChunks => m_WorkerThreadPool => m_GeneratedChunks and m_GeneratedChunkMeshes and m_ChunkStageCache
    //      m_GeneratedChunks => ConsumeGeneratedChunks
    //      m_GeneratedChunkMeshes => ConsumeGeneratedChunkMeshes
    //      m_ChunkStageCache => LoadChunk

    // Define a bunch of chunk generation stage prerequisites.

    struct PrerequisiteData
    {
        ivec3 RelativeChunkPosition;
        ChunkGenerationStage Stage;
        bool Required;

        constexpr PrerequisiteData(ivec3 relativeChunkPosition, ChunkGenerationStage stage, bool required)
            : RelativeChunkPosition(relativeChunkPosition), Stage(stage), Required(required) {}
    };

    // No prerequisites for the first stage, otherwise nothing would ever be generated.
    static constexpr std::array<PrerequisiteData, 0> s_StoneMapPrerequisiteData{};

    static constexpr auto s_TopsoilPrerequisiteData = std::to_array<PrerequisiteData>({
        {{0, 0, 0}, ChunkGenerationStage::Topsoil - 1, true},
        {{0, 1, 0}, ChunkGenerationStage::Topsoil - 1, true},
    });

    // TODO: s_CaveStagePrerequisites

    static constexpr auto s_SurfacePrerequisiteData = std::to_array<PrerequisiteData>({
        {{0, 0, 0}, ChunkGenerationStage::Surface - 1, true},
        {{0, 1, 0}, ChunkGenerationStage::Surface - 1, true},
    });

    // TODO: s_StructureStagePrerequisites
    // TODO: s_FeatureStagePrerequisites
    // TODO: s_FoliageStagePrerequisites

    static constexpr auto s_MeshPrerequisiteData = std::to_array<PrerequisiteData>({
        {{0, 0, 0}, ChunkGenerationStage::Mesh - 1, true},
        {{-1, 0, 0}, ChunkGenerationStage::Mesh - 1, false},
        {{+1, 0, 0}, ChunkGenerationStage::Mesh - 1, false},
        {{0, -1, 0}, ChunkGenerationStage::Mesh - 1, false},
        {{0, +1, 0}, ChunkGenerationStage::Mesh - 1, false},
        {{0, 0, -1}, ChunkGenerationStage::Mesh - 1, false},
        {{0, 0, +1}, ChunkGenerationStage::Mesh - 1, false},
    });

    static constexpr auto s_PrerequisiteData = std::to_array({
        std::span{s_StoneMapPrerequisiteData.data(), s_StoneMapPrerequisiteData.size()},
        std::span{s_TopsoilPrerequisiteData.data(), s_TopsoilPrerequisiteData.size()},
        //std::span{s_CavePrerequisiteData.data(), s_CavePrerequisiteData.size()},
        std::span{s_SurfacePrerequisiteData.data(), s_SurfacePrerequisiteData.size()},
        //std::span{s_StructurePrerequisiteData.data(), s_StructurePrerequisiteData.size()},
        //std::span{s_FeaturePrerequisiteData.data(), s_FeaturePrerequisiteData.size()},
        //std::span{s_FoliagePrerequisiteData.data(), s_FoliagePrerequisiteData.size()},
        std::span{s_MeshPrerequisiteData.data(), s_MeshPrerequisiteData.size()},
    });

    ChunkGenerator::ChunkGenerator(BlockRegistry const& blocks, u8 workerThreadCount)
        : m_Blocks(blocks)
        , m_DelegatorThread([this] { DelegatorThread(); })
    {
        m_WorkerThreads.reserve(workerThreadCount);
        for (u8 i = 0; i < workerThreadCount; i++)
            m_WorkerThreads.emplace_back([this] { WorkerThread(); });
    }

    ChunkGenerator::~ChunkGenerator()
    {
        m_Running.store(false, std::memory_order_relaxed);
        m_GeneratableChunkCondition.notify_all();
        m_QueuedChunkCondition.notify_one();
    }

    template <typename T>
    void ChunkGenerator::Queue(std::span<T> chunks, std::vector<T>& queuedChunks)
    {
        // Enqueue chunks.
        {
            std::unique_lock lock(m_QueuedChunkMutex);
            queuedChunks.append_range(chunks);
        }
        // Tell the delegating thread there's queued chunks to process.
        m_QueuedChunkCondition.notify_one();
    }

    void ChunkGenerator::QueueChunkLoads(std::span<ChunkPos> chunks)
    {
        Queue(chunks, m_QueuedChunkLoads);
    }

    void ChunkGenerator::QueueChunkUnloads(std::span<ChunkPos> chunks)
    {
        Queue(chunks, m_QueuedChunkUnloads);
    }

    void ChunkGenerator::QueueChunkRemeshes(std::span<QueuedChunkRemeshData> chunks)
    {
        Queue(chunks, m_QueuedChunkRemeshes);
    }

    template<typename T>
    void ChunkGenerator::Consume(std::function<void(T&&)> const& consumer, u64 maxCount, std::mutex& mutex, std::vector<T>& output)
    {
        std::unique_lock lock(mutex);
        u64 count = std::min(output.size(), maxCount);
        for (u64 i = 0; i < count; i++)
            consumer(std::move(output[i]));
        output.erase(output.begin(), output.begin() + count);
    }

    void ChunkGenerator::ConsumeGeneratedChunks(std::function<void(std::shared_ptr<Chunk>&&)> const& consumer, u64 maxCount)
    {
        Consume(consumer, maxCount, m_GeneratedChunkMutex, m_GeneratedChunks);
    }

    void ChunkGenerator::ConsumeGeneratedChunkMeshes(std::function<void(ChunkMeshData&&)> const& consumer, u64 maxCount)
    {
        Consume(consumer, maxCount, m_GeneratedChunkMeshMutex, m_GeneratedChunkMeshes);
    }

    void ChunkGenerator::DelegatorThread()
    {
        ThreadTracer tracer("chunk generator delegator");

        while (true)
        {
            // Wait for chunks to be queued or finished generating.
            std::vector<ChunkPos> queuedChunkLoads;
            std::vector<ChunkPos> queuedChunkUnloads;
            std::vector<QueuedChunkRemeshData> queuedChunkRemeshes;
            {
                std::unique_lock lock(m_QueuedChunkMutex);
                m_QueuedChunkCondition.wait(lock);
                if (not m_Running.load(std::memory_order_relaxed))
                    break;
                queuedChunkLoads = std::move(m_QueuedChunkLoads);
                queuedChunkUnloads = std::move(m_QueuedChunkUnloads);
                queuedChunkRemeshes = std::move(m_QueuedChunkRemeshes);
            }

            // Check if all currently pending chunks can now be generated.
            std::erase_if(m_PendingChunks, [this](ChunkStageKey key)
            {
                // Make sure all required prerequisites are satisfied first.
                for (auto& prerequisite : s_PrerequisiteData[key.Stage.Index()])
                {
                    if (prerequisite.Required)
                    {
                        ChunkStageKey prerequisiteKey{key.ChunkPos + prerequisite.RelativeChunkPosition, prerequisite.Stage};
                        bool prerequisiteExists;
                        {
                            std::unique_lock lock(m_ChunkStageCacheMutex);
                            prerequisiteExists = m_ChunkStageCache.contains(prerequisiteKey);
                        }
                        // Keep the chunk pending while its required prerequisites don't exist.
                        if (not prerequisiteExists)
                            return false;
                    }
                }

                // All required prerequisites exist.
                // Move the chunk from m_PendingChunks to m_GeneratableChunks.
                auto prerequisites = GetPrerequisites(key);
                std::unique_lock lock(m_GeneratableChunkMutex);
                m_GeneratableChunks[key].Prerequisites = std::move(prerequisites);
                return true;
            });

            // Process the queued chunk loads.
            for (ChunkPos chunkPos : queuedChunkLoads)
                LoadChunk({chunkPos, ChunkGenerationStage::_End - 1});

            // Process the queued chunk remeshes.
            for (auto& data : queuedChunkRemeshes)
                RemeshChunk(data.ChunkPos, data.UpdatedBlockStates);

            // Tell worker threads that there are generatable chunks.
            {
                bool generatableChunks;
                {
                    std::unique_lock lock(m_GeneratableChunkMutex);
                    generatableChunks = not m_GeneratableChunks.empty();
                }
                if (generatableChunks)
                    m_GeneratableChunkCondition.notify_all();
            }

            // Process the queued chunk unloads.
            for (ChunkPos chunkPos : queuedChunkUnloads)
                UnloadChunk(chunkPos);

            // Unload all unused chunks.
            {
                std::erase_if(m_UnloadingChunks, [this](ChunkPos chunkPos)
                {
                    bool inUse = false;
                    for (ChunkGenerationStage stage : std::span(ChunkGenerationStages).subspan(0, ChunkGenerationStages.size() - 1))
                    {
                        std::unique_lock lock(m_ChunkStageCacheMutex);
                        if (auto it = m_ChunkStageCache.find({chunkPos, stage}); it != m_ChunkStageCache.end())
                        {
                            if (it->second.UsageCount != 0)
                                inUse = true;
                            else
                                m_ChunkStageCache.erase(it);
                        }
                    }
                    return not inUse;
                });
            }
        }
    }

    void ChunkGenerator::WorkerThread()
    {
        ThreadTracer tracer("chunk generator worker");

        while (true)
        {
            // Wait for a generatable chunk to be available.
            ChunkStageKey key;
            GeneratableChunk generatableChunk;
            {
                decltype(m_GeneratableChunks)::node_type node;
                {
                    bool running = true, chunkAvailable = false;
                    std::unique_lock lock(m_GeneratableChunkMutex);
                    while ((running = m_Running.load(std::memory_order_relaxed)) and not (chunkAvailable = not m_GeneratableChunks.empty()))
                        m_GeneratableChunkCondition.wait(lock);

                    if (not running)
                        break;

                    node = m_GeneratableChunks.extract(m_GeneratableChunks.begin());
                }
                key = node.key();
                generatableChunk = std::move(node.mapped());
            }

            // Generate the appropriate stage.
            Timer timer(fmt::format("ChunkGenerator(x={}, y={}, z={}, stage={})", key.ChunkPos.x, key.ChunkPos.y, key.ChunkPos.z, key.Stage.Name()));
            (this->*s_GenerationFunctions[key.Stage.Index()])(key, generatableChunk);
        }
    }

    void ChunkGenerator::LoadChunk(ChunkStageKey key)
    {
        // Make sure all required prerequisites are satisfied first.
        bool allPrerequisitesSatisfied = true;
        for (auto& prerequisite : s_PrerequisiteData[key.Stage.Index()])
        {
            if (prerequisite.Required)
            {
                ChunkStageKey prerequisiteKey{key.ChunkPos + prerequisite.RelativeChunkPosition, prerequisite.Stage};
                bool prerequisiteExists;
                {
                    std::unique_lock lock(m_ChunkStageCacheMutex);
                    prerequisiteExists = m_ChunkStageCache.contains(prerequisiteKey);
                }
                // Load the required prerequisite if it doesn't exist.
                if (not prerequisiteExists)
                {
                    allPrerequisitesSatisfied = false;
                    LoadChunk(prerequisiteKey);
                }
            }
        }

        if (allPrerequisitesSatisfied)
        {
            std::unique_lock lock(m_GeneratableChunkMutex);
            if (not m_GeneratableChunks.contains(key))
            {
                lock.unlock();
                auto prerequisites = GetPrerequisites(key);
                lock.lock();
                m_GeneratableChunks[key].Prerequisites = std::move(prerequisites);
            }
        }
        else
            m_PendingChunks.emplace(key);
    }

    void ChunkGenerator::UnloadChunk(ChunkPos chunkPos)
    {
        m_UnloadingChunks.push_back(chunkPos);
    }

    void ChunkGenerator::RemeshChunk(ChunkPos chunkPos, BlockStateRegistry const* updatedBlockStates)
    {
        // TODO
    }

    auto ChunkGenerator::GetPrerequisites(ChunkStageKey key) -> std::vector<Prerequisite>
    {
        // Get all the prerequisite's block states.
        std::vector<Prerequisite> prerequisites;
        prerequisites.reserve(s_PrerequisiteData[key.Stage.Index()].size());

        for (auto& prerequisite : s_PrerequisiteData[key.Stage.Index()])
        {
            ChunkStageKey prerequisiteKey{key.ChunkPos + prerequisite.RelativeChunkPosition, prerequisite.Stage};
            BlockStateRegistry const* prerequisiteBlockStates = nullptr;
            {
                std::unique_lock lock(m_ChunkStageCacheMutex);
                if (auto it = m_ChunkStageCache.find(prerequisiteKey); it != m_ChunkStageCache.end())
                {
                    it->second.UsageCount++;
                    prerequisiteBlockStates = &it->second.BlockStates;
                }
            }
            prerequisites.emplace_back(prerequisiteKey, prerequisiteBlockStates);
        }

        return prerequisites;
    }

    void ChunkGenerator::GenerateStoneMap(ChunkStageKey key, GeneratableChunk const& data)
    {
        BlockID air = m_Blocks.GetBlock("minecraft:air");
        BlockID stone = m_Blocks.GetBlock("minecraft:stone");

        BlockStateRegistry blockStates;

        for (u16 i = 0; i < Chunk::Size3; i++)
        {
            ivec3 blockPos = ivec3{i, i >> 4, i >> 8} & i32(Chunk::Size - 1);
            BlockID blockID = air;
            if (blockPos.y < 10 and
                0 < blockPos.x and
                0 < blockPos.z)
                blockID = stone;
            blockStates.CreateBlockState(blockID);
        }

        FinishGeneratingStage(key, data, &blockStates);
    }

    void ChunkGenerator::GenerateTopsoil(ChunkStageKey key, GeneratableChunk const& data)
    {
        BlockID air = m_Blocks.GetBlock("minecraft:air");
        BlockID stone = m_Blocks.GetBlock("minecraft:stone");
        BlockID dirt = m_Blocks.GetBlock("minecraft:dirt");

        auto getLastStageBlock = [&data](ivec3 blockOffset)
        {
            BlockStateRegistry const* blockStates = data.Prerequisites[blockOffset.y >= Chunk::Size].BlockStates;
            blockOffset.y %= Chunk::Size;
            BlockStateID id{blockOffset.x + Chunk::Size * blockOffset.y + Chunk::Size2 * blockOffset.z};
            return blockStates->GetComponent<BlockState>(id).BlockID;
        };

        BlockStateRegistry blockStates;

        for (u16 i = 0; i < Chunk::Size3; i++)
        {
            ivec3 blockPos = ivec3{i, i >> 4, i >> 8} & i32(Chunk::Size - 1);

            BlockID blockID = getLastStageBlock(blockPos);
            if (blockID == stone and (
                getLastStageBlock(blockPos + ivec3{0, 4, 0}) == air or
                getLastStageBlock(blockPos + ivec3{0, 3, 0}) == air or
                getLastStageBlock(blockPos + ivec3{0, 2, 0}) == air or
                getLastStageBlock(blockPos + ivec3{0, 1, 0}) == air))
                blockID = dirt;
            blockStates.CreateBlockState(blockID);
        }

        FinishGeneratingStage(key, data, &blockStates);
    }

    void ChunkGenerator::GenerateSurface(ChunkStageKey key, GeneratableChunk const& data)
    {
        BlockID air = m_Blocks.GetBlock("minecraft:air");
        BlockID dirt = m_Blocks.GetBlock("minecraft:dirt");
        BlockID grass = m_Blocks.GetBlock("minecraft:grass");

        auto getLastStageBlock = [&data](ivec3 blockOffset)
        {
            BlockStateRegistry const* blockStates = data.Prerequisites[blockOffset.y >= Chunk::Size].BlockStates;
            blockOffset.y %= Chunk::Size;
            BlockStateID id{blockOffset.x + Chunk::Size * blockOffset.y + Chunk::Size2 * blockOffset.z};
            return blockStates->GetComponent<BlockState>(id).BlockID;
        };

        BlockStateRegistry blockStates;

        for (u16 i = 0; i < Chunk::Size3; i++)
        {
            ivec3 blockPos = ivec3{i, i >> 4, i >> 8} &i32(Chunk::Size - 1);

            BlockID blockID = getLastStageBlock(blockPos);
            if (getLastStageBlock(blockPos) == dirt and getLastStageBlock(blockPos + ivec3{0, 1, 0}) == air)
                blockID = grass;
            blockStates.CreateBlockState(blockID);
        }

        FinishGeneratingStage(key, data, &blockStates);
    }

    void ChunkGenerator::GenerateMesh(ChunkStageKey key, GeneratableChunk const& data)
    {
        BlockStateRegistry const* blockStateRegistry       = data.Prerequisites[0].BlockStates;
        BlockStateRegistry const* leftBlockStateRegistry   = data.Prerequisites[1].BlockStates;
        BlockStateRegistry const* rightBlockStateRegistry  = data.Prerequisites[2].BlockStates;
        BlockStateRegistry const* bottomBlockStateRegistry = data.Prerequisites[3].BlockStates;
        BlockStateRegistry const* topBlockStateRegistry    = data.Prerequisites[4].BlockStates;
        BlockStateRegistry const* backBlockStateRegistry   = data.Prerequisites[5].BlockStates;
        BlockStateRegistry const* frontBlockStateRegistry  = data.Prerequisites[6].BlockStates;

        std::vector<u32> faceSolidMasks(Chunk::Size2 * 6);

        // Represent the solid state of block faces in binary.
        auto blockStates = blockStateRegistry->GetView<BlockState>(); // NOTE: These block states will be used later too.
        for (auto e : blockStates)
        {
            // TODO: variants
            auto& blockState = blockStates.get<BlockState>(e);
            // Only sample block states with a model.
            if (auto* model = m_Blocks.TryGetComponent<BlockModel>(blockState.BlockID))
            {
                // Use the entt::entity/id_type as the block index.
                u32 index = std::to_underlying(e);
                u8 x = index % Chunk::Size;
                u8 y = index / Chunk::Size % Chunk::Size;
                u8 z = index / Chunk::Size2;

                auto constructMask = [&faceSolidMasks, &model = *model](u8 face, u8 px, u8 py, u8 pz)
                {
                    faceSolidMasks[px + Chunk::Size * py + Chunk::Size2 * face] |= (model.SolidBits >> (face ^ 1) & 1) << pz;
                };

                constructMask(0, z, y, x + 1);  // left
                constructMask(1, z, y, 16 - x); // right
                constructMask(2, x, z, y + 1);  // bottom
                constructMask(3, x, z, 16 - y); // top
                constructMask(4, x, y, z + 1);  // back
                constructMask(5, x, y, 16 - z); // front
            }
        }

        ChunkMeshData chunkMeshData
        {
            .ChunkPos = key.ChunkPos,
        };

        // If the whole chunk has no solid faces, there is no mesh.
        // NOTE: this checks if faceSolidMasks is all zeros.
        if (faceSolidMasks.front() != 0 or
            std::memcmp(faceSolidMasks.data(), faceSolidMasks.data() + 1, faceSolidMasks.size() - 1) != 0)
        {
            // TODO: the order { front back bottom left right top } seems optimal for cache (profile).

            auto insertChunkEdge = [this, &faceSolidMasks](BlockStateRegistry const* chunkBlockStateRegistry, u8 face, u8 x, u8 y, u8 z, u8 px, u8 py)
            {
                auto& mask = faceSolidMasks[px + Chunk::Size * py + Chunk::Size2 * face];
                if (chunkBlockStateRegistry)
                {
                    auto blockStates = chunkBlockStateRegistry->GetView<BlockState>();
                    auto e = static_cast<entt::entity>(x + Chunk::Size * (y + Chunk::Size * z));
                    auto& blockState = blockStates.get<BlockState>(e);
                    if (auto* model = m_Blocks.TryGetComponent<BlockModel>(blockState.BlockID))
                        mask |= model->SolidBits >> (face ^ 1) & 1;
                }
                // Treat non-existent chunks as solid.
                else
                    mask |= 1;
            };

            for (u8 py = 0; py < Chunk::Size; py++)
            {
                for (u8 px = 0; px < Chunk::Size; px++)
                {
                    insertChunkEdge(leftBlockStateRegistry,   0, 15, py, px, px, py);
                    insertChunkEdge(rightBlockStateRegistry,  1, 0,  py, px, px, py);
                    insertChunkEdge(bottomBlockStateRegistry, 2, px, 15, py, px, py);
                    insertChunkEdge(topBlockStateRegistry,    3, px, 0,  py, px, py);
                    insertChunkEdge(backBlockStateRegistry,   4, px, py, 15, px, py);
                    insertChunkEdge(frontBlockStateRegistry,  5, px, py, 0,  px, py);
                }
            }

            // Face culling. Yes, it's this simple.
            for (auto& mask : faceSolidMasks)
                mask = u16((mask & ~(mask << 1)) >> 1);

            // Populate greedy meshing planes.
            // TODO: variants, e.g. rotations, will be added to the hashed key type.
            // NOTE: These are stored using std::array since they will already be on the heap from std::unordered_map.
            std::unordered_map<TextureID, std::array<u16, Chunk::Size2 * 6>> greedyMeshingPlanes;

            u16 maskIndex = 0;
            for (u8 face = 0; face < 6; face++)
            {
                for (u8 py = 0; py < Chunk::Size; py++)
                {
                    for (u8 px = 0; px < Chunk::Size; px++)
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
                            auto& model = m_Blocks.GetComponent<BlockModel>(blockState.BlockID);
                            // Put this face in the appropriate greedy meshing plane.
                            TextureID textureID = (&model.Left)[face];
                            greedyMeshingPlanes[textureID][py + Chunk::Size * pz + Chunk::Size2 * face] |= u16(1 << px);
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
                    auto& packedVertices = (&chunkMeshData.Left)[face];
                    for (u8 pz = 0; pz < Chunk::Size; pz++)
                    {
                        auto plane = std::span(facePlanes).subspan(Chunk::Size * pz + Chunk::Size2 * face, Chunk::Size);

                        for (u8 py = 0; py < Chunk::Size; py++)
                        {
                            for (u8 px = 0; px < Chunk::Size; )
                            {
                                // Expand the quad horizontally.
                                px += std::countr_zero<u16>(plane[py] >> px);
                                // If there's no solid faces left, stop advancing.
                                if (px >= Chunk::Size)
                                    break;

                                u8 width = std::countr_one<u16>(plane[py] >> px);
                                u16 widthMask = ((1 << width) - 1) << px;
                                // Consume this mask.
                                plane[py] &= ~widthMask;

                                // Expand the quad vertically.
                                u8 height = 1;
                                while (py + height < Chunk::Size)
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
                                    u16(u16(pz)      | u16(py)      << 4 | u16(px)      << 8), // left
                                    u16(u16(15 - pz) | u16(py)      << 4 | u16(px)      << 8), // right
                                    u16(u16(px)      | u16(pz)      << 4 | u16(py)      << 8), // bottom
                                    u16(u16(px)      | u16(15 - pz) << 4 | u16(py)      << 8), // top
                                    u16(u16(px)      | u16(py)      << 4 | u16(pz)      << 8), // back
                                    u16(u16(px)      | u16(py)      << 4 | u16(15 - pz) << 8), // front
                                });
                                u16 index = planeToLocal[face];
                                u8 x = index % Chunk::Size;
                                u8 y = index / Chunk::Size % Chunk::Size;
                                u8 z = index / Chunk::Size2;

                                // Pack the quad into the appropriate submesh.
                                packedVertices.push_back(packVertex(textureID, {x, y, z, width, height}));

                                px += width;
                            }
                        }
                    }
                }
            }
        }

        FinishGeneratingChunkMesh(std::move(chunkMeshData));
        FinishGeneratingStage(key, data, nullptr);
    }

    void ChunkGenerator::FinishGeneratingStage(ChunkStageKey key, GeneratableChunk const& data, BlockStateRegistry* blockStates)
    {
        if (blockStates)
        {
            if (key.Stage == ChunkGenerationStage::Mesh - 1)
                FinishGeneratingChunk(key, *blockStates);

            // Add the generated stage to the cache.
            {
                std::unique_lock lock(m_ChunkStageCacheMutex);
                ENG_ASSERT(not m_ChunkStageCache.contains(key));
                m_ChunkStageCache[key].BlockStates = std::move(*blockStates);
            }
        }

        // Mark this chunk as no longer using its prerequisites.
        for (auto& prerequisite : data.Prerequisites)
        {
            if (prerequisite.BlockStates)
            {
                std::unique_lock lock(m_ChunkStageCacheMutex);
                m_ChunkStageCache[prerequisite.Key].UsageCount--;
            }
        }

        // Tell the delegator thread it can recheck pending chunks.
        m_QueuedChunkCondition.notify_one();
    }

    void ChunkGenerator::FinishGeneratingChunk(ChunkStageKey key, BlockStateRegistry const& blockStates)
    {
        // Add the generated chunk to the output.
        std::unique_lock lock(m_GeneratedChunkMutex);
        // Make a copy of the block states. The original will be used for the cache.
        m_GeneratedChunks.push_back(std::make_shared<Chunk>(m_Blocks, BlockStateRegistry{blockStates}, key.ChunkPos));
    }

    void ChunkGenerator::FinishGeneratingChunkMesh(ChunkMeshData&& chunkMeshData)
    {
        // Add the generated mesh to the output.
        std::unique_lock lock(m_GeneratedChunkMeshMutex);
        m_GeneratedChunkMeshes.push_back(std::move(chunkMeshData));
    }

    u64 ChunkGenerator::ChunkStageHashEq::operator()(ChunkStageKey const& key) const noexcept
    {
        u64 hashS = std::hash<i8>{}(+key.Stage);
        return (hashS << 48 | hashS >> 16) ^ ChunkPosHash{}(key.ChunkPos);
    }

    bool ChunkGenerator::ChunkStageHashEq::operator()(ChunkStageKey const& lhs, ChunkStageKey const& rhs) const noexcept
    {
        return lhs.ChunkPos == rhs.ChunkPos and lhs.Stage == rhs.Stage;
    }
}

/*

        BlockID air = m_Blocks.GetBlock("minecraft:air");
        BlockID bedrock = m_Blocks.GetBlock("minecraft:bedrock");
        BlockID stone = m_Blocks.GetBlock("minecraft:stone");
        BlockID dirt = m_Blocks.GetBlock("minecraft:dirt");
        BlockID grass = m_Blocks.GetBlock("minecraft:grass");

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
                    blockStates.CreateBlockState(blockID);
                }
            }
        }

*/
