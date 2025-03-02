#pragma once

#include <vulkan/vulkan.h>
#include <span>

namespace eng
{
    class RenderContext;

    struct VertexBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        std::uint64_t Size = 0; // In bytes
    };

    class VertexBuffer
    {
    public:
        VertexBuffer(VertexBufferInfo const& info);
        ~VertexBuffer();

        void SetData(std::span<std::uint8_t const> data);

        template <class Container>
        requires(std::is_convertible_v<Container, std::span<std::add_const_t<typename Container::value_type>>>)
        void SetData(Container const& container)
        {
            std::span<std::add_const_t<typename Container::value_type>> data = container;
            SetData({(std::uint8_t const*)data.data(), data.size_bytes()});
        }

        void Bind(VkCommandBuffer commandBuffer, std::uint32_t binding = 0);
    private:

        std::uint32_t SelectMemoryType(std::uint32_t memoryType, VkMemoryPropertyFlags properties);
    private:
        RenderContext& m_Context; // non-owning
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
    };
}
