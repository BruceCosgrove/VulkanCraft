#pragma once

#include "VulkanCraft/World/BlockStateRegistry.hpp"
#include "VulkanCraft/World/ChunkPos.hpp"
#include "VulkanCraft/World/ChunkGenerationStage.hpp"
#include <Engine.hpp>
#include <any>
#include <optional>

using namespace eng;

namespace vc
{
    class World;
    class WorldRenderer;

    class Chunk : public std::enable_shared_from_this<Chunk>
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Chunk);
    public:
        // TODO: refactor constants
        inline static constexpr u32 Size = 16;
        inline static constexpr u32 Size2 = Size * Size;
        inline static constexpr u32 Size3 = Size * Size * Size;
    public:
        Chunk(World& world, BlockRegistry& blockRegistry, ChunkPos position);
        ~Chunk();

        ChunkPos GetPosition() const;
    private:
        friend class World;
        friend class WorldRenderer;

        void GenerateTerrain();
        void GenerateMesh();

        ChunkGenerationStage GetGenerationStage() const;
        bool IsGenerating() const;

        template <typename T>
        std::optional<T> ConsumeGenerationStageOutput()
        {
            if (T* casted = (T*)std::any_cast<T>(&m_StageOutput))
            {
                T output = std::move(*casted);
                m_StageOutput.reset();
                return output;
            }
            return std::nullopt;
        }
    private:
        World& m_World; // non-owning
        BlockRegistry& m_BlockRegistry; // non-owning
        BlockStateRegistry m_BlockStateRegistry;
        ChunkPos m_Position;

        ChunkGenerationStage m_GenerationStage;
        std::atomic_bool m_Generating;
        std::any m_StageOutput;
    };
}
