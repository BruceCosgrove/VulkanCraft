#include "Framebuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    Framebuffer::Framebuffer(FramebufferInfo const& info)
        : m_Context(*info.RenderContext)
    {
        VkExtent2D extent = m_Context.GetSwapchainExtent();

        VkFramebufferCreateInfo framebufferInfo
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = info.RenderPass,
            .attachmentCount = static_cast<u32>(info.Attachments.size()),
            .pAttachments = info.Attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1, // TODO
        };
        VkResult result = vkCreateFramebuffer(m_Context.GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer.");
    }

    Framebuffer::~Framebuffer()
    {
        vkDestroyFramebuffer(m_Context.GetDevice(), m_Framebuffer, nullptr);
    }

    VkFramebuffer Framebuffer::GetFramebuffer() const
    {
        return m_Framebuffer;
    }
}
