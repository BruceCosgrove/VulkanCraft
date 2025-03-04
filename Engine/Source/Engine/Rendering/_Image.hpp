#pragma once

#include "Engine/Rendering/_Buffer.hpp"

namespace eng
{
    class RenderContext;
}

namespace eng::detail
{
    // Helper class for common image functionality.
    class Image : protected detail::Buffer
    {
    protected:
        Image(RenderContext* context);
        ~Image() = default;

        void CreateImage(
            VkImageType type,
            VkFormat format,
            VkExtent3D extent,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags flags,
            VkImage& image,
            VkDeviceMemory& deviceMemory
        );

        void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkExtent3D extent);
    };
}
