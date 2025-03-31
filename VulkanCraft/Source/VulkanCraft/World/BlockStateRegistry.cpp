#include "BlockStateRegistry.hpp"

namespace vc
{
    BlockStateID BlockStateRegistry::CreateBlockState(BlockID id)
    {
        entt::entity e = m_Registry.create();
        m_Registry.emplace<BlockState>(e, id);
        return static_cast<BlockStateID>(e);
    }
}
