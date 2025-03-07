#pragma once

#include "Engine/Rendering/Image.hpp"

namespace eng
{
    struct FramebufferAttachmentInfo
    {
        RenderContext* RenderContext = nullptr;
        VkExtent2D Extent{};
        VkFormat Format = VK_FORMAT_UNDEFINED;
        VkImageUsageFlags Usage = 0;
        VkImageAspectFlags Aspect = 0;
        VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    class FramebufferAttachment : public Image
    {
    public:
        FramebufferAttachment(FramebufferAttachmentInfo const& info);
    };
}
