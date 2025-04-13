#pragma once

#include "VulkanCraft/World/BlockState.hpp"
#include <entt/entt.hpp>

namespace vc
{
    class BlockStateRegistry
    {
    public:
        BlockStateRegistry() = default;
        BlockStateRegistry(BlockStateRegistry const& blockStateRegistry);
        BlockStateRegistry(BlockStateRegistry&&) noexcept = default;
        BlockStateRegistry& operator=(BlockStateRegistry const& blockStateRegistry);
        BlockStateRegistry& operator=(BlockStateRegistry&&) noexcept = default;

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
        T const& GetComponent(BlockStateID id) const
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

        template <class T>
        T const* TryGetComponent(BlockStateID id) const
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.try_get<T>(e);
        }

        template <class... T>
        auto GetView()
        {
            return m_Registry.view<T...>();
        }

        template <class... T>
        auto GetView() const
        {
            return m_Registry.view<T...>();
        }

        entt::registry& Get();

        entt::registry const& Get() const;
    private:
        entt::registry m_Registry;
    };
}
