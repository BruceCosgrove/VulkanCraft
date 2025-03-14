#include "VertexBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    VertexBuffer::VertexBuffer(VertexBufferInfo const& info)
        : m_Context(*info.RenderContext)
        , m_Size(info.Size)
    {
        BufferUtils::CreateBuffer(
            m_Context,
            m_Size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );
        BufferUtils::MapMemory(m_Context, m_DeviceMemory, 0, m_Size, m_MappedMemory);
    }

    VertexBuffer::~VertexBuffer()
    {
        VkDevice device = m_Context.GetDevice();

        BufferUtils::UnmapMemory(m_Context, m_DeviceMemory);

        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void VertexBuffer::SetData(std::span<u8 const> data)
    {
        ENG_ASSERT(data.size() <= m_Size);
        std::memcpy(m_MappedMemory, data.data(), data.size());
    }

    void VertexBuffer::Bind(VkCommandBuffer commandBuffer, u32 binding)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, binding, 1, &m_Buffer, &offset);
    }
}
