#pragma once

#include <vulkan/vulkan.h>
#include <span>

namespace eng
{
    class RenderContext;

    struct RenderPassInfo
    {
        RenderContext* RenderContext = nullptr;
        std::span<VkAttachmentDescription> Attachments;
        std::span<VkSubpassDescription> Subpasses;
        std::span<VkSubpassDependency> SubpassDependencies;
    };

    class RenderPass
    {
    public:
        RenderPass(RenderPassInfo const& info);
        ~RenderPass();

        VkRenderPass GetRenderPass() const;
    private:
        RenderContext& m_Context; // non-owning
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    };
}
