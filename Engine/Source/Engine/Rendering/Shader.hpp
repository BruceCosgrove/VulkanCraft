#pragma once

#include <vulkan/vulkan.h>
#include <filesystem>
#include <span>
#include <vector>

namespace eng
{
    class RenderContext;
    class UniformBuffer;

    struct VertexBinding
    {
        std::uint32_t Binding = 0;
        VkVertexInputRate InputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        std::initializer_list<std::uint32_t> Locations;
    };

    struct ShaderInfo
    {
        RenderContext* RenderContext = nullptr;
        std::filesystem::path Filepath;
        std::initializer_list<VertexBinding> VertexBindings;
        VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
        std::vector<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> CompileExistingSources(
            std::filesystem::path const& filepath
        );

        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageInfos(
            std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages
        );

        void Reflect(
            ShaderInfo const& info,
            std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages,
            std::vector<std::uint32_t>& strides,
            std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions,
            std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings,
            std::vector<VkDescriptorPoolSize>& descriptorPoolSizes
        );

        void CreateDescriptorSetLayout(std::span<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings);
        void CreateDescriptorPool(std::span<VkDescriptorPoolSize> descriptorPoolSizes);
        void CreateDescriptorSets();
        void CreatePipelineLayout();

        void CreatePipeline(
            ShaderInfo const& info,
            std::span<std::uint32_t> strides,
            std::span<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions,
            std::span<VkPipelineShaderStageCreateInfo> pipelineShaderStageInfos
        );
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
