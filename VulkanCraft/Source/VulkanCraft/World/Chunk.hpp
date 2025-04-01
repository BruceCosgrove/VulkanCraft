#pragma once

#include "VulkanCraft/Rendering/ChunkMesh.hpp"
#include "VulkanCraft/World/BlockStateRegistry.hpp"
#include <Engine.hpp>

using namespace eng;

namespace vc
{
    class Chunk : public std::enable_shared_from_this<Chunk>
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Chunk);
    public:
        // TODO: refactor constants
        inline static constexpr u32 Size = 16;
        inline static constexpr u32 Size2 = Size * Size;
        inline static constexpr u32 Size3 = Size * Size * Size;
    public:
        Chunk(ivec3 position, RenderContext& context);
        ~Chunk();
    public: // TODO: private: friend class World;
        void GenerateTerrain();
        void GenerateMesh();

        DynamicResource<std::shared_ptr<ChunkMesh>>& GetMesh();
    private:
        static u64 Index(u8 x, u8 y, u8 z);
    private:
        ivec3 m_Position;

        BlockStateRegistry m_BlockStateRegistry;

        // Rendering
        RenderContext& m_Context; // non-owning
        DynamicResource<std::shared_ptr<ChunkMesh>> m_Mesh;
    };
}
