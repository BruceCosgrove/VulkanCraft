#pragma once

#include "VulkanCraft/Rendering/ChunkMeshData.hpp"
#include "VulkanCraft/World/BlockRegistry.hpp"
#include "VulkanCraft/World/BlockStateRegistry.hpp"
#include "VulkanCraft/World/ChunkGenerationStage.hpp"
#include "VulkanCraft/World/ChunkPos.hpp"
#include <Engine.hpp>
#include <array>
#include <atomic>
#include <functional>
#include <mutex>
#include <semaphore>
#include <span>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace eng;

namespace vc
{
    class Chunk;

    class ChunkGenerator
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(ChunkGenerator);
    public:
        struct QueuedChunkRemeshData
        {
            ChunkPos ChunkPos;                            // The position to update a chunk mesh.
            BlockStateRegistry const* UpdatedBlockStates; // The block states to mesh with (makes a local copy).
        };
    public:
        ChunkGenerator(BlockRegistry const& blocks, u8 workerThreadCount);
        ~ChunkGenerator();

        // Queues the chunks at the given positions to be loaded.
        void QueueChunkLoads(std::span<ChunkPos> chunks);
        // Queues the chunks at the given positions to be unloaded.
        void QueueChunkUnloads(std::span<ChunkPos> chunks);
        // Queues the chunks at the given positions to be remeshed with the given block states.
        void QueueChunkRemeshes(std::span<QueuedChunkRemeshData> chunks);

        void ConsumeGeneratedChunks(std::function<void(std::shared_ptr<Chunk>&&)> const& consumer, u64 maxCount = u64(-1));
        void ConsumeGeneratedChunkMeshes(std::function<void(ChunkMeshData&&)> const& consumer, u64 maxCount = u64(-1));
    private:
        // Cache

        struct ChunkStageKey
        {
            ChunkPos ChunkPos{};
            ChunkGenerationStage Stage;
        };
        struct ChunkStageData
        {
            BlockStateRegistry BlockStates;
            u64 UsageCount = 0;
        };
        struct ChunkStageHashEq
        {
            ENG_NO_DISCARD u64 operator()(ChunkStageKey const& key) const noexcept;
            ENG_NO_DISCARD bool operator()(ChunkStageKey const& lhs, ChunkStageKey const& rhs) const noexcept;
        };
        using ChunkStageCache = std::unordered_map<ChunkStageKey, ChunkStageData, ChunkStageHashEq, ChunkStageHashEq>;

        // Data

        struct Prerequisite
        {
            ChunkStageKey Key;
            BlockStateRegistry const* BlockStates;
        };

        struct GeneratableChunk
        {
            std::vector<Prerequisite> Prerequisites;
        };
    private:
        template <typename T>
        void Queue(std::span<T> chunks, std::vector<T>& queuedChunks);

        template <typename T>
        void Consume(std::function<void(T&&)> const& consumer, u64 maxCount, std::mutex& mutex, std::vector<T>& output);

        void DelegatorThread();
        void WorkerThread();
        void LoadChunk(ChunkStageKey key);
        void UnloadChunk(ChunkPos chunkPos);
        void RemeshChunk(ChunkPos chunkPos, BlockStateRegistry const* updatedBlockStates);
        std::vector<Prerequisite> GetPrerequisites(ChunkStageKey key);
    private:
        void GenerateStoneMap(ChunkStageKey key, GeneratableChunk const& data);
        void GenerateTopsoil(ChunkStageKey key, GeneratableChunk const& data);
        //void GenerateCaves(ChunkStageKey key, GeneratableChunk const& data);
        void GenerateSurface(ChunkStageKey key, GeneratableChunk const& data);
        //void GenerateStructures(ChunkStageKey key, GeneratableChunk const& data);
        //void GenerateFeatures(ChunkStageKey key, GeneratableChunk const& data);
        //void GenerateFoliage(ChunkStageKey key, GeneratableChunk const& data);
        void GenerateMesh(ChunkStageKey key, GeneratableChunk const& data);

        static constexpr auto s_GenerationFunctions = std::to_array({
            &ChunkGenerator::GenerateStoneMap,
            &ChunkGenerator::GenerateTopsoil,
            //&ChunkGenerator::GenerateCaves,
            &ChunkGenerator::GenerateSurface,
            //&ChunkGenerator::GenerateStructures,
            //&ChunkGenerator::GenerateFeatures,
            //&ChunkGenerator::GenerateFoliage,
            &ChunkGenerator::GenerateMesh,
        });

        void FinishGeneratingStage(ChunkStageKey key, GeneratableChunk const& data, BlockStateRegistry* blockStates);
        void FinishGeneratingChunk(ChunkStageKey key, BlockStateRegistry const& blockStates);
        void FinishGeneratingChunkMesh(ChunkMeshData&& meshData);
    private:
        BlockRegistry const& m_Blocks; // non-owning

        // Input chunks to load/unload/remesh.
        std::vector<ChunkPos> m_QueuedChunkLoads;
        std::vector<ChunkPos> m_QueuedChunkUnloads;
        std::vector<QueuedChunkRemeshData> m_QueuedChunkRemeshes;
        std::mutex m_QueuedChunkMutex;
        std::condition_variable m_QueuedChunkCondition;

        // Chunks to unload when they are no longer used.
        std::vector<ChunkPos> m_UnloadingChunks;

        // Chunks to load, and their prerequisites.
        std::unordered_set<ChunkStageKey, ChunkStageHashEq, ChunkStageHashEq> m_PendingChunks;

        // Chunks to load whose prerequisites are satisfied.
        std::unordered_map<ChunkStageKey, GeneratableChunk, ChunkStageHashEq, ChunkStageHashEq> m_GeneratableChunks;
        std::mutex m_GeneratableChunkMutex;
        std::condition_variable m_GeneratableChunkCondition;

        // Loaded chunks.
        std::vector<std::shared_ptr<Chunk>> m_GeneratedChunks;
        std::mutex m_GeneratedChunkMutex;

        // Loaded chunk meshes.
        std::vector<ChunkMeshData> m_GeneratedChunkMeshes;
        std::mutex m_GeneratedChunkMeshMutex;

        // Cache of intermediate chunk generation block states.
        ChunkStageCache m_ChunkStageCache;
        std::mutex m_ChunkStageCacheMutex;

        // Flag for if the threads should continue running.
        std::atomic_bool m_Running = true;
        // Threads that generate chunk stages.
        std::vector<std::jthread> m_WorkerThreads;
        // Thread that figures out what chunks stages should be generated and in what order.
        std::jthread m_DelegatorThread;
    };
}
