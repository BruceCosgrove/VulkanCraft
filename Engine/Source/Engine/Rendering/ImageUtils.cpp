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
        VkMemoryPropertyFlags properties,
        VkImageAspectFlags aspect,
        VkImage& image,
        VkImageView& imageView,
        VkDeviceMemory& deviceMemory
    )
    {
        VkDevice device = context.GetDevice();

        // Create the image.
        {
            VkImageCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = type,
                .format = format,
                .extent = extent,
                .mipLevels = 1, // TODO
                .arrayLayers = layerCount,
                .samples = VK_SAMPLE_COUNT_1_BIT, // TODO
                .tiling = VK_IMAGE_TILING_OPTIMAL, // TODO
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // TODO
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // TODO
            };
            VkResult result = vkCreateImage(device, &info, nullptr, &image);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create image.");
        }

        // Allocate the memory.
        {
            // Get memory requirements.
            VkMemoryRequirements memoryRequirements;
            vkGetImageMemoryRequirements(device, image, &memoryRequirements);

            // TODO: use https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
            VkMemoryAllocateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memoryRequirements.size,
                .memoryTypeIndex = BufferUtils::SelectMemoryType(memoryRequirements.memoryTypeBits, properties),
            };
            VkResult result = vkAllocateMemory(device, &info, nullptr, &deviceMemory);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate image memory.");
        }

        // Bind the memory to the image.
        {
            VkResult result = vkBindImageMemory(device, image, deviceMemory, 0);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to bind image memory.");
        }

        // Create the image view.
        {
            VkImageViewCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = viewType,
                .format = format,
                .subresourceRange
                {
                    .aspectMask = aspect,
                    .baseMipLevel = 0, // TODO
                    .levelCount = 1, // TODO
                    .baseArrayLayer = 0, // TODO
                    .layerCount = layerCount,
                },
            };
            VkResult result = vkCreateImageView(device, &info, nullptr, &imageView);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create image view.");
        }
    }

    void ImageUtils::TransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        u32 layerCount
    )
    {
        VkImageMemoryBarrier barrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange
            {
                .baseMipLevel = 0, // TODO
                .levelCount = 1, // TODO
                .baseArrayLayer = 0, // TODO
                .layerCount = layerCount,
            },
        };

        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_NONE;

        //// RenderContext needs to transition images from color attachment to shader read only.
        //// Guaranteed to happen every frame.
        //if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        //{
        //    // TODO: no idea if this is correct, but it seems to work.
        //    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    barrier.srcAccessMask = VK_ACCESS_NONE;
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
        //    barrier.dstAccessMask = VK_ACCESS_NONE;
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
            barrier.srcAccessMask = VK_ACCESS_NONE;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        // Might happen every frame.
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            barrier.srcAccessMask = VK_ACCESS_NONE;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        // Texture2D needs to transition images from undefined to transfer destination.
        // Seldom happens.
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = VK_ACCESS_NONE;
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
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // TODO: not all image (samplers) will be in fragment shaders.
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: see above todo.
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
        VkBufferImageCopy region
        {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // TODO
                .mipLevel = 0, // TODO
                .baseArrayLayer = 0, // TODO
                .layerCount = layerCount,
            },
            .imageOffset{0, 0, 0},
            .imageExtent = extent,
        };
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }
}
