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
            u32 mipmapLevels,
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
            u32 layerCount,
            u32 mipmapLevels
        );

        static void CopyBufferToImage(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            VkImage image,
            VkExtent3D extent,
            u32 layerCount
        );

        // Assumes the image is already in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
        // TODO: results in all image mipmap levels being in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
        static void GenerateMipmaps(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkExtent3D extent,
            u32 layerCount,
            u32 mipmapLevels
        );
    };
}
