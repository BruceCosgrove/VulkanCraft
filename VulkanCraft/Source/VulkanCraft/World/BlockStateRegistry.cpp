#include "BlockStateRegistry.hpp"

namespace vc
{
    BlockStateRegistry::BlockStateRegistry(BlockStateRegistry const& blockStateRegistry)
    {
        // TODO: Figure out a more effecient way of copying an entt::registry.
        // That way must preserve entt::entity id values.
        auto view = blockStateRegistry.GetView<BlockState>();
        for (u16 i = 0; i < view.size(); i++)
            CreateBlockState(view.get<BlockState>(entt::entity{i}).BlockID);
    }

    BlockStateRegistry& BlockStateRegistry::operator=(BlockStateRegistry const& blockStateRegistry)
    {
        if (this != std::addressof(blockStateRegistry)) ENG_LIKELY
        {
            m_Registry.clear();

            // TODO: Figure out a more effecient way of copying an entt::registry.
            // That way must preserve entt::entity id values.
            auto view = blockStateRegistry.GetView<BlockState>();
            for (u16 i = 0; i < view.size(); i++)
                CreateBlockState(view.get<BlockState>(entt::entity{i}).BlockID);
        }
        return *this;
    }

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
