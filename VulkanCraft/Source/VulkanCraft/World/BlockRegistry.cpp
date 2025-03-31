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
        m_Registry.emplace<Block>(e, small_string(id));
        return static_cast<BlockID>(e);
    }
}
