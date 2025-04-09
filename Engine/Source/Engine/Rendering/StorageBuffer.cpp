#include "StorageBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    StorageBuffer::StorageBuffer(StorageBufferInfo const& info)
        : m_Context(*info.RenderContext)
        , m_Size(info.Size)
    {
        // Account for alignment.
        VkDeviceSize minAlignment = m_Context.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
        m_Size = BufferUtils::Align(m_Size, minAlignment);

        VkDeviceSize totalSize = m_Size * m_Context.GetSwapchainImageCount();

        BufferUtils::CreateBuffer(
            m_Context,
            totalSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Buffer,
            m_DeviceMemory
        );

        void* mappedMemory = nullptr;
        BufferUtils::MapMemory(m_Context, m_DeviceMemory, 0, totalSize, mappedMemory);
        m_MappedMemory = std::span((u8*)mappedMemory, totalSize);
    }

    StorageBuffer::~StorageBuffer()
    {
        VkDevice device = m_Context.GetDevice();

        BufferUtils::UnmapMemory(m_Context, m_DeviceMemory);

        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void StorageBuffer::SetData(std::span<u8 const> data)
    {
        ENG_ASSERT(data.size() <= m_Size);

        std::span frameMemory = m_MappedMemory.subspan(GetOffset(), m_Size);
        std::memcpy(frameMemory.data(), data.data(), data.size());
    }

    VkDeviceSize StorageBuffer::GetOffset() const
    {
        return m_Size * m_Context.GetSwapchainImageIndex();
    }

    VkDeviceSize StorageBuffer::GetSize() const
    {
        return m_Size;
    }

    VkBuffer StorageBuffer::GetBuffer() const
    {
        return m_Buffer;
    }
}
