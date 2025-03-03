#pragma once

#include <vulkan/vulkan.h>
#include <filesystem>
#include <span>
#include <unordered_map>
#include <vector>

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
        void UpdateDescriptorSet();

        UniformBuffer* GetUniformBuffer(std::uint32_t binding);
    private:
        std::vector<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> CompileExistingSources(std::filesystem::path const& filepath);
        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageInfos(std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages);
        void Reflect(
            std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages,
            std::uint32_t& stride,
            std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions,
            std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings,
            std::vector<VkDescriptorPoolSize>& descriptorPoolSizes
        );

        void CreateDescriptorSetLayout(std::span<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings);
        void CreateDescriptorPool(std::span<VkDescriptorPoolSize> descriptorPoolSizes);
        void CreateDescriptorSets();
    private:
        RenderContext& m_Context; // non-owning
        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;

        // Shader resources.

        template <typename T>
        struct ResourceData
        {
            std::uint32_t Binding = 0;
            std::unique_ptr<T> Resource;
        };

        struct FrameData
        {
            // NOTE: Assuming one descriptor set per frame.
            // Realistically, there can/should be up to four,
            // but this is the simplest option right now.
            VkDescriptorSet DescriptorSet;
            std::vector<ResourceData<UniformBuffer>> UniformBuffers;
            // TODO: storage buffers
            // TODO: image samplers
        };
        std::vector<FrameData> m_FrameData;
    };
}
