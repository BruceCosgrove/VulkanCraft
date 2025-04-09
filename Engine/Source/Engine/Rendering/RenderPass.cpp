#include "RenderPass.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    RenderPass::RenderPass(RenderPassInfo const& info)
        : m_Context(*info.RenderContext)
    {
        VkRenderPassCreateInfo renderPassInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = u32(info.Attachments.size()),
            .pAttachments = info.Attachments.data(),
            .subpassCount = u32(info.Subpasses.size()),
            .pSubpasses = info.Subpasses.data(),
            .dependencyCount = u32(info.SubpassDependencies.size()),
            .pDependencies = info.SubpassDependencies.data(),
        };
        VkResult result = vkCreateRenderPass(m_Context.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create render pass.");
    }

    RenderPass::~RenderPass()
    {
        vkDestroyRenderPass(m_Context.GetDevice(), m_RenderPass, nullptr);
    }

    VkRenderPass RenderPass::GetRenderPass() const
    {
        return m_RenderPass;
    }
}
