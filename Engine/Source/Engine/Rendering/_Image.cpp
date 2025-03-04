#include "_Image.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng::detail
{
    Image::Image(RenderContext* context)
        : Buffer(context)
    {

    }

    void Image::CreateImage(
        VkImageType type,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags flags,
        VkImage& image,
        VkDeviceMemory& deviceMemory
    )
    {
        VkDevice device = m_Context.GetDevice();

        // Create the image.
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = type;
        imageInfo.format = format;
        imageInfo.extent = extent;
        imageInfo.mipLevels = 1; // TODO
        imageInfo.arrayLayers = 1; // TODO
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO

        VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create image.");

        // Get memory requirements.
        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(device, image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = Buffer::SelectMemoryType(memoryRequirements.memoryTypeBits, flags);

        // Allocate the memory.
        // TODO: use https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate image memory.");

        // Bind the memory to the image.
        result = vkBindImageMemory(device, image, deviceMemory, 0);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to bind image memory.");
    }

    void Image::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
        barrier.subresourceRange.baseMipLevel = 0; // TODO
        barrier.subresourceRange.levelCount = 1; // TODO
        barrier.subresourceRange.baseArrayLayer = 0; // TODO
        barrier.subresourceRange.layerCount = 1; // TODO

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: not all image (samplers) will be in fragment shaders.
        }
        else
            ENG_ASSERT(false, "Invalid image layout transition.");

        vkCmdPipelineBarrier(
            commandBuffer,
            srcStage,
            dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void Image::CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkExtent3D extent)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
        region.imageSubresource.mipLevel = 0; // TODO
        region.imageSubresource.baseArrayLayer = 0; // TODO
        region.imageSubresource.layerCount = 1; // TODO
        region.imageOffset = {0, 0, 0};
        region.imageExtent = extent;

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }
}
