#include "Chunk.hpp"

namespace vc
{
    Chunk::Chunk(BlockRegistry const& blocks, BlockStateRegistry&& blockStates, ChunkPos position)
        : m_Blocks(blocks)
        , m_BlockStates(std::move(blockStates))
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
}
