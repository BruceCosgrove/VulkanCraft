#include "VertexBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    VertexBuffer::VertexBuffer(VertexBufferInfo const& info)
        : Buffer(info.RenderContext)
        , m_Size(info.Size)
    {
        Buffer::CreateBuffer(
            m_Size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );
        Buffer::MapMemory(m_DeviceMemory, 0, m_Size, m_MappedMemory);
    }

    VertexBuffer::~VertexBuffer()
    {
        VkDevice device = m_Context.GetDevice();

        Buffer::UnmapMemory(m_DeviceMemory);

        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void VertexBuffer::SetData(std::span<std::uint8_t const> data)
    {
        ENG_ASSERT(data.size() <= m_Size);
        std::memcpy(m_MappedMemory, data.data(), data.size());
    }

    void VertexBuffer::Bind(VkCommandBuffer commandBuffer, std::uint32_t binding)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, binding, 1, &m_Buffer, &offset);
    }
}
