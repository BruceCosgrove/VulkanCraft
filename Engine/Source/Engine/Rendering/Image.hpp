#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class RenderContext;

    class Image
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Image);
    public:
        Image(RenderContext* context);
        ~Image();

        VkImage GetImage() const;
        VkImageView GetImageView() const;
    protected:
        RenderContext& m_Context; // non-owning
        VkImage m_Image = nullptr;
        VkDeviceMemory m_DeviceMemory = nullptr;
        VkImageView m_ImageView = nullptr;
    };
}
