#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <vulkan/vulkan.h>
#include <span>

namespace eng
{
    class RenderContext;

    struct FramebufferInfo
    {
        RenderContext* RenderContext = nullptr;
        VkRenderPass RenderPass = nullptr;
        std::span<VkImageView> Attachments;
    };

    class Framebuffer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Framebuffer);
    public:
        Framebuffer(FramebufferInfo const& info);
        ~Framebuffer();

        VkFramebuffer GetFramebuffer() const;
    private:
        RenderContext& m_Context; // non-owning
        VkFramebuffer m_Framebuffer = nullptr;
    };
}
