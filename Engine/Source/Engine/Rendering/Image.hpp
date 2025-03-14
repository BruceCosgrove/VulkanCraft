#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class RenderContext;

    class Image
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Image);

        Image(RenderContext* context);
        ~Image();

        VkImage GetImage() const;
        VkImageView GetImageView() const;
    protected:
        RenderContext& m_Context; // non-owning
        VkImage m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
    };
}
