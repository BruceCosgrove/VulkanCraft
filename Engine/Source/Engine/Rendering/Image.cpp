#include "Engine/Rendering/Image.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    Image::Image(RenderContext* context)
        : m_Context(*context)
    {

    }

    Image::~Image()
    {
        VkDevice device = m_Context.GetDevice();

        vkDestroyImageView(device, m_ImageView, nullptr);
        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyImage(device, m_Image, nullptr);
    }

    VkImage Image::GetImage() const
    {
        return m_Image;
    }

    VkImageView Image::GetImageView() const
    {
        return m_ImageView;
    }
}
