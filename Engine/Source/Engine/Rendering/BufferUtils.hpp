#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include <vulkan/vulkan.h>
#include <ranges>

namespace eng
{
    class RenderContext;

    // Helper class for common buffer functionality.
    class BufferUtils
    {
    public:
        ENG_STATIC_CLASS(BufferUtils);

        static void CreateBuffer(
            RenderContext& context,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags flags,
            VkBuffer& buffer,
            VkDeviceMemory& deviceMemory
        );

        static void MapMemory(
            RenderContext& context,
            VkDeviceMemory deviceMemory,
            VkDeviceSize offset,
            VkDeviceSize size,
            void*& mappedMemory
        );

        static void UnmapMemory(RenderContext& context, VkDeviceMemory deviceMemory);

        static u32 SelectMemoryType(u32 memoryType, VkMemoryPropertyFlags flags);
    };
}

#define _ENG_BUFFER_SET_ARBITRARY_DATA(setDataFunc) \
    template <typename Container> \
    void setDataFunc(Container const& container) \
    { \
        if constexpr (::std::ranges::range<Container>) \
        { \
            static_assert(::std::is_convertible_v<Container, ::std::span<::std::add_const_t<typename Container::value_type>>>); \
            ::std::span<::std::add_const_t<typename Container::value_type>> data = container; \
            setDataFunc({(::eng::u8 const*)data.data(), data.size_bytes()}); \
        } \
        else \
            setDataFunc({(::eng::u8 const*)::std::addressof(container), sizeof(container)}); \
    }
