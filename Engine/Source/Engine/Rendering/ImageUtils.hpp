#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
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
        );

        static void TransitionImageLayout(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            u32 layerCount
        );

        static void CopyBufferToImage(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            VkImage image,
            VkExtent3D extent,
            u32 layerCount
        );
    };
}
