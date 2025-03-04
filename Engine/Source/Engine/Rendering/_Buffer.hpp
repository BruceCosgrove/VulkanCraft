#pragma once

#include <vulkan/vulkan.h>
#include <concepts>

namespace eng
{
    class RenderContext;
}

namespace eng::detail
{
    // Helper class for common buffer functionality.
    class Buffer
    {
    protected:
        Buffer(RenderContext* context);
        ~Buffer() = default;

        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags flags,
            VkBuffer& buffer,
            VkDeviceMemory& deviceMemory
        );

        void MapMemory(
            VkDeviceMemory deviceMemory,
            VkDeviceSize offset,
            VkDeviceSize size,
            void*& mappedMemory
        );

        void UnmapMemory(VkDeviceMemory deviceMemory);

        static std::uint32_t SelectMemoryType(std::uint32_t memoryType, VkMemoryPropertyFlags flags);

    protected:
        RenderContext& m_Context; // non-owning
    };
}

#define _ENG_BUFFER_SET_ARBITRARY_DATA(setDataFunc) \
    template <typename Container> \
    void setDataFunc(Container const& container) \
    { \
        /* TODO: this is not foolproof. */ \
        constexpr bool isContainer = requires() \
        { \
            { container.size() } -> ::std::same_as<::std::size_t>; \
        }; \
        if constexpr (isContainer) \
        { \
            static_assert(::std::is_convertible_v<Container, ::std::span<::std::add_const_t<typename Container::value_type>>>); \
            ::std::span<::std::add_const_t<typename Container::value_type>> data = container; \
            setDataFunc({(::std::uint8_t const*)data.data(), data.size_bytes()}); \
        } \
        else \
            setDataFunc({(::std::uint8_t const*)::std::addressof(container), sizeof(container)}); \
    }
