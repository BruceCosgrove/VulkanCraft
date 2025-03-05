#include "UniformBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    UniformBuffer::UniformBuffer(UniformBufferInfo const& info)
        : Buffer(info.RenderContext)
        , m_Size(info.Size)
    {
        VkDeviceSize minAlignment = m_Context.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        VkDeviceSize misalignmentCorrection = m_Size % minAlignment ? minAlignment : 0;
        m_AlignedSize = (m_Size / minAlignment) * minAlignment + misalignmentCorrection;

        VkDeviceSize totalSize = m_AlignedSize * m_Context.GetSwapchainImageCount();

        Buffer::CreateBuffer(
            totalSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );

        void* mappedMemory;
        Buffer::MapMemory(m_DeviceMemory, 0, m_Size, mappedMemory);

        m_MappedMemory = std::span(static_cast<std::uint8_t*>(mappedMemory), totalSize);
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

        std::span frameMemory = m_MappedMemory.subspan(GetOffset(), m_Size);
        std::memcpy(frameMemory.data(), data.data(), data.size());
    }

    VkDeviceSize UniformBuffer::GetOffset() const
    {
        return m_AlignedSize * m_Context.GetSwapchainImageIndex();
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
