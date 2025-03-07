#pragma once

#include <vulkan/vulkan.h>
#include <span>

namespace eng
{
    class RenderContext;

    struct FramebufferInfo
    {
        RenderContext* RenderContext = nullptr;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
        std::span<VkImageView> Attachments;
    };

    class Framebuffer
    {
    public:
        Framebuffer(FramebufferInfo const& info);
        ~Framebuffer();

        VkFramebuffer GetFramebuffer() const;
    private:
        RenderContext& m_Context; // non-owning
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    };
}
