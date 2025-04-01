#include "BlockRegistry.hpp"

namespace vc
{
    BlockRegistry::BlockRegistry()
    {
        CreateBlock("minecraft:void");
    }

    BlockID BlockRegistry::CreateBlock(small_string_view id)
    {
        entt::entity e = m_Registry.create();
        Block& block = m_Registry.emplace<Block>(e, small_string(id));
        BlockID blockID = static_cast<BlockID>(e);
        // Let m_IDs use string views as its key, and keep the host
        // string alive as long as the block with that ID exists.
        m_IDs[block.ID] = blockID;
        return blockID;
    }

    BlockID BlockRegistry::GetBlock(small_string_view id)
    {
        if (auto it = m_IDs.find(id); it != m_IDs.end())
            return it->second;
        return BlockID(0); // minecraft:void
    }
}
