#pragma once

#include "VulkanCraft/World/Chunk.hpp"
#include "VulkanCraft/World/ChunkPos.hpp"
#include <Engine.hpp>
#include <memory>
#include <unordered_map>

using namespace eng;

namespace vc
{
    class World
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(World);
    private:
        std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> m_Chunks;
    };
}
