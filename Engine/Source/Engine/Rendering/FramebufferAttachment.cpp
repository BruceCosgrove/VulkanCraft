#include "FramebufferAttachment.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include "Engine/Rendering/ImageUtils.hpp"

namespace eng
{
    FramebufferAttachment::FramebufferAttachment(FramebufferAttachmentInfo const& info)
        : Image(info.RenderContext)
    {
        ImageUtils::CreateImage(
            m_Context,
            VK_IMAGE_TYPE_2D,
            info.Format,
            {info.Extent.width, info.Extent.height, 1},
            info.Usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // TODO
            info.Aspect,
            m_Image,
            m_ImageView,
            m_DeviceMemory
        );

        VkCommandBuffer commandBuffer = m_Context.BeginOneTimeCommandBuffer();
        // Transition the image layout to be optimal for this attachment.
        ImageUtils::TransitionImageLayout(commandBuffer, m_Image, info.Format, VK_IMAGE_LAYOUT_UNDEFINED, info.Layout);
        m_Context.EndOneTimeCommandBuffer(commandBuffer);
    }
}
