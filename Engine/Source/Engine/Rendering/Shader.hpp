#pragma once

#include <vulkan/vulkan.h>
#include <filesystem>
#include <span>
#include <unordered_map>

namespace eng
{
    class RenderContext;
    class UniformBuffer;

    struct ShaderInfo
    {
        RenderContext* RenderContext = nullptr;
        std::filesystem::path Filepath;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
    };

    // TODO: separate shader from pipeline.
    // The pipeline can be recreated without recreating the shader.
    class Shader
    {
    public:
        Shader(ShaderInfo const& info);
        ~Shader();
    public:
        void Bind(VkCommandBuffer commandBuffer);
        void UpdateDescriptorSets();

        UniformBuffer& GetUniformBuffer(std::uint32_t binding);
    private:
        void CreateDescriptorSetLayout(std::span<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings);
        void CreateDescriptorPool(std::span<VkDescriptorPoolSize> descriptorPoolSizes);
        void CreateDescriptorSets();
    private:
        RenderContext& m_Context; // non-owning
        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;

        // Shader resources.

        // Map from bindings to per-frame uniform buffers.
        // TODO: this *really* needs to be improved in terms of capabilities.
        std::unordered_map<std::uint32_t, std::vector<std::unique_ptr<UniformBuffer>>> m_UniformBuffers;
        // TODO: storage buffers
        // TODO: image samplers
    };
}
