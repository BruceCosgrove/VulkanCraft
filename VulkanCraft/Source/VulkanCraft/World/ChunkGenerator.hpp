#pragma once

#include "VulkanCraft/Rendering/ChunkMeshData.hpp"
#include "VulkanCraft/World/BlockRegistry.hpp"
#include "VulkanCraft/World/BlockStateRegistry.hpp"
#include "VulkanCraft/World/ChunkGenerationStage.hpp"
#include "VulkanCraft/World/ChunkPos.hpp"
#include <Engine.hpp>
#include <array>
#include <atomic>
#include <deque>
#include <functional>
#include <limits>
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
        struct QueuedChunk
        {
            ChunkPos ChunkPos; // The position to load or unload a chunk.
            bool Load;         // Whether to load or unload the chunk.
        };
    public:
        ChunkGenerator(BlockRegistry const& blocks, u8 workerThreadCount);
        ~ChunkGenerator();

        // Queues the chunks at the given positions to be loaded or unloaded.
        void QueueChunks(std::span<QueuedChunk> queuedChunks);

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
            std::atomic<u64> UsageCount;
        };
        struct ChunkStageHashEq
        {
            ENG_NO_DISCARD u64 operator()(ChunkStageKey const& key) const noexcept;
            ENG_NO_DISCARD bool operator()(ChunkStageKey const& lhs, ChunkStageKey const& rhs) const noexcept;
        };
        using ChunkStageCache = std::unordered_map<ChunkStageKey, ChunkStageData, ChunkStageHashEq, ChunkStageHashEq>;

        // Data

        using UnloadingChunk = ChunkStageKey;

        using PendingChunk = ChunkStageKey;

        struct GeneratableChunk
        {
            std::vector<BlockStateRegistry const*> Prerequisites;
        };
    private:
        void DelegatorThread();
        void WorkerThread();
        void LoadChunk(ChunkStageKey key);
        void UnloadChunk(ChunkPos chunkPos);
        std::vector<BlockStateRegistry const*> GetPrerequisiteBlockStates(ChunkStageKey key);
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

        void FinishGeneratingStage(ChunkStageKey key, BlockStateRegistry&& blockStates);
        void FinishGeneratingChunk(ChunkStageKey key, BlockStateRegistry const& blockStates);
        void FinishGeneratingChunkMesh(ChunkMeshData&& meshData);
    private:
        // Chunk flow:
        //  QueueChunks => m_QueuedChunks => m_DelegatorThread
        //      m_DelegatorThread => LoadChunk or UnloadChunk
        //          LoadChunk => m_PendingChunks or m_GeneratableChunks
        //              m_PendingChunks => m_GeneratableChunks
        //              m_GeneratableChunks => m_WorkerThreadPool => m_GeneratedChunks and m_GeneratedChunkMeshes and m_ChunkStageCache
        //                  m_GeneratedChunks => ConsumeGeneratedChunks
        //                  m_GeneratedChunkMeshes => ConsumeGeneratedChunkMeshes
        //                  m_ChunkStageCache => LoadChunk

        BlockRegistry const& m_Blocks; // non-owning

        // Input chunks to load/unload.
        std::deque<QueuedChunk> m_QueuedChunks;
        std::mutex m_QueuedChunkMutex;
        std::condition_variable m_QueuedChunkCondition;

        // Chunks to unload.
        std::vector<ChunkStageKey> m_UnloadingChunks;

        // Chunks to load, and their prerequisites, ordered with prerequisites first.
        std::unordered_set<UnloadingChunk, ChunkStageHashEq, ChunkStageHashEq> m_PendingChunks;

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

        std::atomic_bool m_Running = true;
        std::jthread m_DelegatorThread;
        std::vector<std::jthread> m_WorkerThreads;

        ChunkStageCache m_ChunkStageCache;
        std::mutex m_ChunkStageCacheMutex;
    };
}
