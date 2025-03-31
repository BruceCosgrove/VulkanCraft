#pragma once

#include <Engine.hpp>
#include <entt/entt.hpp>

using namespace eng;

namespace vc
{
    enum class BlockID : u32 {};

    struct Block
    {
        small_string ID;
        // TODO: variants
    };

    class BlockRegistry
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(BlockRegistry);
    public:
        BlockRegistry();
    public:
        BlockID CreateBlock(small_string_view id);

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
