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
        ENG_STATIC_CLASS(BufferUtils);
    public:
        static void CreateBuffer(
            RenderContext& context,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
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

        static u32 SelectMemoryType(u32 memoryType, VkMemoryPropertyFlags properties);

        template <std::unsigned_integral T1, std::unsigned_integral T2>
        static constexpr auto Align(T1 size, T2 alignment)
        {
            using CT = std::common_type_t<T1, T2>;
            return CT((CT(size) + CT(alignment) - 1) & ~(CT(alignment) - 1));
        }
    };
}

#define _ENG_BUFFER_SET_DATA(setDataFunc) \
    template <typename Container> \
    void setDataFunc(Container const& container) \
    { \
        if constexpr (::std::is_convertible_v<Container, ::std::span<::std::add_const_t<typename Container::value_type>>>) \
        { \
            ::std::span<::std::add_const_t<typename Container::value_type>> data = container; \
            setDataFunc({(::eng::u8 const*)data.data(), data.size_bytes()}); \
        } \
        else \
            setDataFunc({(::eng::u8 const*)::std::addressof(container), sizeof(Container)}); \
    } \
    void setDataFunc(::std::span<u8 const> data)

