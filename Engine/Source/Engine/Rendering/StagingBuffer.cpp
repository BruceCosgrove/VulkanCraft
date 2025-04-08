#include "StagingBuffer.hpp"
#include "Engine/Rendering/BufferUtils.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    StagingBuffer::StagingBuffer(StagingBufferInfo const& info)
        : m_Context(*info.RenderContext)
    {
        BufferUtils::CreateBuffer(
            m_Context,
            info.Size,
            info.Usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );
        BufferUtils::MapMemory(m_Context, m_DeviceMemory, 0, info.Size, m_MappedMemory);
    }

    StagingBuffer::~StagingBuffer()
    {
        BufferUtils::UnmapMemory(m_Context, m_DeviceMemory);
        m_Context.DeferFree([&context = m_Context, buffer = m_Buffer, deviceMemory = m_DeviceMemory]
        {
            VkDevice device = context.GetDevice();
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, deviceMemory, nullptr);
        });
    }

    void* StagingBuffer::GetMappedMemory()
    {
        return m_MappedMemory;
    }

    VkBuffer StagingBuffer::GetBuffer() const
    {
        return m_Buffer;
    }
}
