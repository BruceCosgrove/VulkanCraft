#pragma once

#include "VulkanCraft/Rendering/ChunkMesh.hpp"
#include "VulkanCraft/World/BlockStateRegistry.hpp"
#include "VulkanCraft/World/ChunkPos.hpp"
#include <Engine.hpp>

using namespace eng;

namespace vc
{
    class World;

    class Chunk : public std::enable_shared_from_this<Chunk>
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Chunk);
    public:
        // TODO: refactor constants
        inline static constexpr u32 Size = 16;
        inline static constexpr u32 Size2 = Size * Size;
        inline static constexpr u32 Size3 = Size * Size * Size;
    public:
        Chunk(World& world, BlockRegistry& blockRegistry, ChunkPos position, RenderContext& context);
        ~Chunk();
    private:
        friend class World;
        void GenerateTerrain();
        void GenerateMesh();

        DynamicResource<std::shared_ptr<ChunkMesh>>& GetMesh();
    private:
        World& m_World; // non-owning
        BlockRegistry& m_BlockRegistry; // non-owning
        BlockStateRegistry m_BlockStateRegistry;
        ChunkPos m_Position;

        // Rendering
        RenderContext& m_Context; // non-owning
        DynamicResource<std::shared_ptr<ChunkMesh>> m_Mesh;
    };
}
