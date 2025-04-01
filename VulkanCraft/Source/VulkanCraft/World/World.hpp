#pragma once

#include "VulkanCraft/World/Chunk.hpp"
#include <Engine.hpp>
#include <memory>
#include <unordered_map>

using namespace eng;

namespace vc
{
    class World
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(World);
    public:
        World(RenderContext& context);
    public:
        Chunk* GetChunk(ChunkPos chunkPos);

        // TODO: Separate rendering code. Moreover, this way of drawing will be replaced entirely.
        void Draw(VkCommandBuffer commandBuffer);
    private:
        RenderContext& m_Context; // non-owning
        BlockRegistry m_BlockRegistry;
        std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> m_Chunks;
    };
}
