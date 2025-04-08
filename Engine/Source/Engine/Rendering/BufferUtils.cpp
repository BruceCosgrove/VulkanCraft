#include "BufferUtils.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    void BufferUtils::CreateBuffer(
        RenderContext& context,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& deviceMemory
    )
    {
        VkDevice device = context.GetDevice();

        // Create the buffer.
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create buffer.");

        // Get memory requirements.
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = SelectMemoryType(memoryRequirements.memoryTypeBits, properties);

        // Allocate the memory.
        // TODO: use https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate buffer memory.");

        // Bind the memory to the buffer.
        result = vkBindBufferMemory(device, buffer, deviceMemory, 0);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to bind buffer memory.");
    }

    void BufferUtils::MapMemory(
        RenderContext& context,
        VkDeviceMemory deviceMemory,
        VkDeviceSize offset,
        VkDeviceSize size, 
        void*& mappedMemory
    )
    {
        VkResult result = vkMapMemory(context.GetDevice(), deviceMemory, offset, size, 0, &mappedMemory);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to map buffer memory.");
    }

    void BufferUtils::UnmapMemory(RenderContext& context, VkDeviceMemory deviceMemory)
    {
        vkUnmapMemory(context.GetDevice(), deviceMemory);
    }

    u32 BufferUtils::SelectMemoryType(u32 memoryType, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDevice physicalDevice = RenderContext::GetPhysicalDevice();

        VkPhysicalDeviceMemoryProperties deviceProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceProperties);

        for (u32 i = 0; i < deviceProperties.memoryTypeCount; i++)
            if ((memoryType & (1 << i)) and (deviceProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        ENG_ASSERT(false, "Failed to find memory type.");
        return 0;
    }
}
