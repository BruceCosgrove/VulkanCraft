#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class RenderContext;

    // Helper class for common image functionality.
    class ImageUtils
    {
    public:
        ENG_STATIC_CLASS(ImageUtils);

        static void CreateImage(
            RenderContext& context,
            VkImageType type,
            VkFormat format,
            VkExtent3D extent,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags flags,
            VkImageAspectFlags aspect,
            VkImage& image,
            VkImageView& imageView,
            VkDeviceMemory& deviceMemory
        );

        static void TransitionImageLayout(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout
        );

        static void CopyBufferToImage(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            VkImage image,
            VkExtent3D extent
        );
    };
}
