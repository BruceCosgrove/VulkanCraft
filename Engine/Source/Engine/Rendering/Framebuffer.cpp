#include "Framebuffer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    Framebuffer::Framebuffer(FramebufferInfo const& info)
        : m_Context(*info.RenderContext)
    {
        VkExtent2D extent = m_Context.GetSwapchainExtent();
        VkDevice device = m_Context.GetDevice();

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = info.RenderPass;
        framebufferInfo.attachmentCount = static_cast<std::uint32_t>(info.Attachments.size());
        framebufferInfo.pAttachments = info.Attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1; // TODO

        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer);
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
