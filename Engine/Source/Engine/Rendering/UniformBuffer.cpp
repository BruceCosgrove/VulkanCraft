#include "UniformBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    UniformBuffer::UniformBuffer(UniformBufferInfo const& info)
        : m_Context(*info.RenderContext)
        , m_Size(info.Size)
    {
        // Account for alignment.
        VkDeviceSize minAlignment = m_Context.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        VkDeviceSize misalignmentCorrection = m_Size % minAlignment ? minAlignment : 0;
        // Modify m_Size directly to allow it to use the extra storage.
        m_Size = (m_Size / minAlignment) * minAlignment + misalignmentCorrection;

        VkDeviceSize totalSize = m_Size * m_Context.GetSwapchainImageCount();

        BufferUtils::CreateBuffer(
            m_Context,
            totalSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );

        void* mappedMemory;
        BufferUtils::MapMemory(m_Context, m_DeviceMemory, 0, m_Size, mappedMemory);

        m_MappedMemory = std::span(static_cast<u8*>(mappedMemory), totalSize);
    }

    UniformBuffer::~UniformBuffer()
    {
        VkDevice device = m_Context.GetDevice();

        BufferUtils::UnmapMemory(m_Context, m_DeviceMemory);

        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void UniformBuffer::SetData(std::span<u8 const> data)
    {
        ENG_ASSERT(data.size() <= m_Size);

        std::span frameMemory = m_MappedMemory.subspan(GetOffset(), m_Size);
        std::memcpy(frameMemory.data(), data.data(), data.size());
    }

    VkDeviceSize UniformBuffer::GetOffset() const
    {
        return m_Size * m_Context.GetSwapchainImageIndex();
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
