#pragma once

#include "VulkanCraft/World/BlockRegistry.hpp"

namespace vc
{
    enum class BlockStateID : u32 {};

    struct BlockState
    {
        BlockID BlockID{};
        // TODO: variants
    };

    class BlockStateRegistry
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_DEFAULTABLE_CLASS(BlockStateRegistry);
    public:
        BlockStateID CreateBlockState(BlockID id);

        template <class T, typename... Args>
        T& EmplaceComponent(BlockID id, Args&&... args)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.emplace<T>(e, std::forward<Args>(args)...);
        }
    private:
        entt::registry m_Registry;
    };
}
