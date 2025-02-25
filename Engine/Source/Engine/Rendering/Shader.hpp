#pragma once

#include <vulkan/vulkan.h>
#include <filesystem>

namespace eng
{
    class RenderContext;

    struct ShaderInfo
    {
        RenderContext* RenderContext;
        std::filesystem::path Filepath;
        VkRenderPass RenderPass;
    };

    class Shader
    {
    public:
        Shader(ShaderInfo const& info);
        ~Shader();
    public:
        void Bind(VkCommandBuffer commandBuffer);
    private:
        RenderContext& m_Context; // non-owning
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
    };
}
