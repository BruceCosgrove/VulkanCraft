#pragma once

#include "VulkanCraft/World/Block.hpp"
#include <entt/entt.hpp>

using namespace eng;

namespace vc
{
    class BlockRegistry
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(BlockRegistry);
    public:
        BlockRegistry();

        BlockID CreateBlock(small_string_view id);
        BlockID GetBlock(small_string_view id) const;

        template <class T, typename... Args>
        T& EmplaceComponent(BlockID id, Args&&... args)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.emplace<T>(e, std::forward<Args>(args)...);
        }

        template <class T>
        bool HasComponent(BlockID id) const
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.all_of<T>(e);
        }

        template <class T>
        T& GetComponent(BlockID id)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.get<T>(e);
        }

        template <class T>
        T const& GetComponent(BlockID id) const
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.get<T>(e);
        }

        template <class T>
        T* TryGetComponent(BlockID id)
        {
            auto e = static_cast<entt::entity>(id);
            return m_Registry.try_get<T>(e);
        }

        template <class T>
        T const* TryGetComponent(BlockID id) const
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
    private:
        entt::registry m_Registry;
        std::unordered_map<small_string_view, BlockID> m_IDs;
    };
}
