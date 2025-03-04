#include "UniformBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    UniformBuffer::UniformBuffer(UniformBufferInfo const& info)
        : Buffer(info.RenderContext)
        , m_Size(info.Size)
    {
        Buffer::CreateBuffer(
            m_Size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );
        Buffer::MapMemory(m_DeviceMemory, 0, m_Size, m_MappedMemory);
    }

    UniformBuffer::~UniformBuffer()
    {
        VkDevice device = m_Context.GetDevice();

        Buffer::UnmapMemory(m_DeviceMemory);

        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void UniformBuffer::SetData(std::span<std::uint8_t const> data)
    {
        ENG_ASSERT(data.size() <= m_Size);
        std::memcpy(m_MappedMemory, data.data(), data.size());
    }

    VkDeviceSize UniformBuffer::GetSize() const
    {
        return m_Size;
    }

    VkBuffer UniformBuffer::GetBuffer() const
    {
        return m_Buffer;
    }
}
