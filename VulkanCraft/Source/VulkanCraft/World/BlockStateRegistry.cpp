#include "BlockStateRegistry.hpp"

namespace vc
{
    BlockStateID BlockStateRegistry::CreateBlockState(BlockID id)
    {
        entt::entity e = m_Registry.create();
        m_Registry.emplace<BlockState>(e, id);
        return static_cast<BlockStateID>(e);
    }

    entt::registry& BlockStateRegistry::Get()
    {
        return m_Registry;
    }

    entt::registry const& BlockStateRegistry::Get() const
    {
        return m_Registry;
    }
}
