#include "ImageUtils.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/BufferUtils.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    void ImageUtils::CreateImage(
        RenderContext& context,
        VkImageType type,
        VkImageViewType viewType,
        VkFormat format,
        VkExtent3D extent,
        u32 layerCount,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags flags,
        VkImageAspectFlags aspect,
        VkImage& image,
        VkImageView& imageView,
        VkDeviceMemory& deviceMemory
    )
    {
        VkDevice device = context.GetDevice();

        // Create the image.
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = type;
        imageInfo.format = format;
        imageInfo.extent = extent;
        imageInfo.mipLevels = 1; // TODO
        imageInfo.arrayLayers = layerCount;
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
        memoryAllocateInfo.memoryTypeIndex = BufferUtils::SelectMemoryType(memoryRequirements.memoryTypeBits, flags);

        // Allocate the memory.
        // TODO: use https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate image memory.");

        // Bind the memory to the image.
        result = vkBindImageMemory(device, image, deviceMemory, 0);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to bind image memory.");

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = image;
        imageViewInfo.viewType = viewType;
        imageViewInfo.format = format;
        //imageViewInfo.components = // TODO
        imageViewInfo.subresourceRange.aspectMask = aspect;
        imageViewInfo.subresourceRange.baseMipLevel = 0; // TODO
        imageViewInfo.subresourceRange.levelCount = 1; // TODO
        imageViewInfo.subresourceRange.baseArrayLayer = 0; // TODO
        imageViewInfo.subresourceRange.layerCount = layerCount;

        result = vkCreateImageView(device, &imageViewInfo, nullptr, &imageView);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create image view.");
    }

    void ImageUtils::TransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        u32 layerCount
    )
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0; // TODO
        barrier.subresourceRange.levelCount = 1; // TODO
        barrier.subresourceRange.baseArrayLayer = 0; // TODO
        barrier.subresourceRange.layerCount = layerCount;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        //// RenderContext needs to transition images from color attachment to shader read only.
        //// Guaranteed to happen every frame.
        //if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        //{
        //    // TODO: no idea if this is correct, but it seems to work.
        //    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    barrier.srcAccessMask = 0;
        //    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        //    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        //}
        //// RenderContext needs to transition images from shader read only to color attachment.
        //// Guaranteed to happen every frame.
        //else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        //{
        //    // TODO: no idea if this is correct, but it seems to work.
        //    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //    barrier.dstAccessMask = 0;
        //    srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        //    dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        //}
        //else

        // Might happen every frame.
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            // TODO: fairly sure this is correct because it follows the depth buffer
            // example directly below from https://vulkan-tutorial.com/en/Depth_buffering.
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        // Might happen every frame.
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        // Texture2D needs to transition images from undefined to transfer destination.
        // Seldom happens.
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        // Texture2D needs to transition images from transfer destination to shader read only.
        // Seldom happens.
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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

    void ImageUtils::CopyBufferToImage(
        VkCommandBuffer commandBuffer,
        VkBuffer buffer,
        VkImage image,
        VkExtent3D extent,
        u32 layerCount
    )
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
        region.imageSubresource.mipLevel = 0; // TODO
        region.imageSubresource.baseArrayLayer = 0; // TODO
        region.imageSubresource.layerCount = layerCount;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = extent;

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }
}
