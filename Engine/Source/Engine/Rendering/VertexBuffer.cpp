#include "VertexBuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    VertexBuffer::VertexBuffer(VertexBufferInfo const& info)
        : m_Context(*info.RenderContext)
    {
        VkDevice device = m_Context.GetDevice();

        // Create the buffer.
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = info.Size;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create vertex buffer.");

        // Get memory requirements.
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, m_Buffer, &memoryRequirements);

        std::uint32_t memoryTypeIndex = SelectMemoryType(
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

        // Allocate the memory.
        // TODO: use https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &m_DeviceMemory);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate buffer memory.");

        // Bind the memory to the buffer.
        result = vkBindBufferMemory(device, m_Buffer, m_DeviceMemory, 0);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to bind buffer memory.");
    }

    VertexBuffer::~VertexBuffer()
    {
        VkDevice device = m_Context.GetDevice();

        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void VertexBuffer::SetData(std::span<std::uint8_t const> data)
    {
        VkDevice device = m_Context.GetDevice();

        void* mappedMemory;
        VkResult result = vkMapMemory(device, m_DeviceMemory, 0, data.size(), 0, &mappedMemory);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to map buffer memory.");
        std::memcpy(mappedMemory, data.data(), data.size());
        vkUnmapMemory(device, m_DeviceMemory);
    }

    void VertexBuffer::Bind(VkCommandBuffer commandBuffer, std::uint32_t binding)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, binding, 1, &m_Buffer, &offset);
    }

    std::uint32_t VertexBuffer::SelectMemoryType(std::uint32_t memoryType, VkMemoryPropertyFlags flags)
    {
        VkPhysicalDevice physicalDevice = RenderContext::GetPhysicalDevice();

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

        for (std::uint32_t i = 0; i < properties.memoryTypeCount; i++)
            if ((memoryType & (1 << i)) and (properties.memoryTypes[i].propertyFlags & flags) == flags)
                return i;

        ENG_ASSERT(false, "Failed to find memory type.");
        return 0;
    }
}
