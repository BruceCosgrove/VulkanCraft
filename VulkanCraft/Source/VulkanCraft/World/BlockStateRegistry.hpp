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
        T& EmplaceComponent(BlockStateID id, Args&&... args)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.emplace<T>(e, std::forward<Args>(args)...);
        }

        template <class T>
        T& GetComponent(BlockStateID id)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.get<T>(e);
        }

        template <class T>
        T* TryGetComponent(BlockStateID id)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.try_get<T>(e);
        }

        template <class... T>
        auto GetView()
        {
            return m_Registry.view<T...>();
        }
    private:
        entt::registry m_Registry;
    };
}
